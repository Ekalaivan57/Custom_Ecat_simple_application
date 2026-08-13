#include "EthercatManager.h"
#include "define.h"
#include <stdio.h>
#include <unistd.h>

/* Global Handles */
ecat_master_t g_master = NULL;
ecat_domain_t g_domain = NULL;

ecat_slave_t  g_slave_ek1100 = NULL;
ecat_slave_t  g_slave_el5101 = NULL;
ecat_slave_t  g_slave_el5072 = NULL;
ecat_slave_t  g_slave_el4008 = NULL;
ecat_slave_t  g_slave_el2004 = NULL;

ecat_pdo_t    g_pdo_el5101_val = NULL;

ecat_pdo_t    g_pdo_el5072_cmd = NULL;
ecat_pdo_t    g_pdo_el5072_val = NULL;
ecat_pdo_t    g_pdo_el5072_pos = NULL;

ecat_pdo_t    g_pdo_el4008_ao[8] = {NULL};

ecat_pdo_t    g_pdo_el2004_do[4] = {NULL};

/* Calibrated zero offset for LVDT */
static int32_t g_lvdt_zero = (int32_t)LVDT_HOMING_OFFSET;

bool init_ethercat(void)
{
    printf("===================================================================\n");
    printf(" ERL Spectra EtherCAT Application Initializing\n");
    printf(" Target Slaves: EK1100, EL5101, EL5072, EL4008, EL2004\n");
    printf("===================================================================\n\n");

    /* Step 1: Request Master Index 0 */
    g_master = ecat_req_master(0);
    if (!g_master) {
        fprintf(stderr, "[ERROR] Failed to request EtherCAT Master index 0.\n");
        return false;
    }

    /* Step 2: Create Process Data Domain */
    g_domain = ecat_create_domain(g_master);
    if (!g_domain) {
        fprintf(stderr, "[ERROR] Failed to create EtherCAT domain.\n");
        return false;
    }

    /* Step 3: Explicit Slave Configurations */
    printf("[CONFIG] Configuring bus slaves...\n");
    g_slave_ek1100 = ecat_slave_config(g_master, EK1100_ALIAS, EK1100_POS, BECKHOFF_VENDOR_ID, EK1100_PRODUCT);
    g_slave_el5101 = ecat_slave_config(g_master, EL5101_ALIAS, EL5101_POS, BECKHOFF_VENDOR_ID, EL5101_PRODUCT);
    g_slave_el5072 = ecat_slave_config(g_master, EL5072_ALIAS, EL5072_POS, BECKHOFF_VENDOR_ID, EL5072_PRODUCT);
    g_slave_el4008 = ecat_slave_config(g_master, EL4008_ALIAS, EL4008_POS, BECKHOFF_VENDOR_ID, EL4008_PRODUCT);
    g_slave_el2004 = ecat_slave_config(g_master, EL2004_ALIAS, EL2004_POS, BECKHOFF_VENDOR_ID, EL2004_PRODUCT);

    if (!g_slave_ek1100 || !g_slave_el5101 || !g_slave_el5072 || !g_slave_el4008 || !g_slave_el2004) {
        fprintf(stderr, "[ERROR] Failed to configure one or more slaves on the bus.\n");
        return false;
    }

    /* Step 4: Register PDO Entries */
    printf("[PDO-REG] Registering Slave PDO Entries...\n");

    // Position 1: EL5101 Incremental Encoder PDO
    g_pdo_el5101_val = ecat_pdo_reg(g_slave_el5101, 0x6000, 0x02, 16);

    // Position 2: EL5072 LVDT PDOs
    g_pdo_el5072_cmd = ecat_pdo_reg(g_slave_el5072, 0x7000, 0x01, 1);
    g_pdo_el5072_val = ecat_pdo_reg(g_slave_el5072, 0x7000, 0x11, 32);
    g_pdo_el5072_pos = ecat_pdo_reg(g_slave_el5072, 0x6001, 0x01, 32);

    // Position 3: EL4008 8-Ch Analog Output (0-10V) PDOs
    for (int ch = 0; ch < 8; ch++) {
        uint16_t index = 0x7000 + (ch * 0x10);
        g_pdo_el4008_ao[ch] = ecat_pdo_reg(g_slave_el4008, index, 0x01, 16);
    }

    // Position 4: EL2004 4-Ch Digital Output (24V) PDOs
    g_pdo_el2004_do[0] = ecat_pdo_reg(g_slave_el2004, 0x7000, 0x01, 1);
    g_pdo_el2004_do[1] = ecat_pdo_reg(g_slave_el2004, 0x7010, 0x01, 1);
    g_pdo_el2004_do[2] = ecat_pdo_reg(g_slave_el2004, 0x7020, 0x01, 1);
    g_pdo_el2004_do[3] = ecat_pdo_reg(g_slave_el2004, 0x7030, 0x01, 1);

    /* Step 5: Master Activation */
    int act_ret = ecat_activate(g_master);
    if (act_ret < 0) {
        fprintf(stderr, "[ERROR] Master activation failed.\n");
        return false;
    }

    printf("[SUCCESS] Application Initialized! Real-time EtherCAT communication active.\n\n");
    return true;
}

void LVDT_Zero_Set(int val)
{
    g_lvdt_zero = val;
    printf("[CALIBRATION] Zero Homing Offset set to: %d counts\n", g_lvdt_zero);
}

float getLVDTpos(void)
{
    if (!g_pdo_el5072_pos) return 0.0f;
    int32_t raw_counts = ecat_rd_s32(g_pdo_el5072_pos);
    int32_t calibrated_counts = raw_counts - g_lvdt_zero;
    return (float)(calibrated_counts * LVDT_GAIN_SCALE);
}

void Ethercat_Run(bool *running)
{
    uint32_t cycle_count = 0;
    bool ch1_do_state = false;
    bool ch2_do_state = false;
    bool ch3_do_state = true;
    bool ch4_do_state = false;

    /* Target Analog Output Voltage Variable (Change this anytime!) */
    float target_ao_voltage = 5.0f; 

    while (*running) {
        // Step 1: Receive Ethernet frames
        ecat_recv(g_master);

        // Step 2: Read Encoder & LVDT Sensors
        uint16_t encoder_counts = g_pdo_el5101_val ? ecat_rd_u16(g_pdo_el5101_val) : 0;
        int32_t  raw_lvdt       = g_pdo_el5072_pos ? ecat_rd_s32(g_pdo_el5072_pos) : 0;
        float    displacement_mm = getLVDTpos();

        // Step 3: Command Digital Outputs
        if (cycle_count % 500 == 0) {
            ch1_do_state = !ch1_do_state;
        }

        if (g_pdo_el2004_do[0]) ecat_wr_bit(g_pdo_el2004_do[0], ch1_do_state);
        if (g_pdo_el2004_do[1]) ecat_wr_bit(g_pdo_el2004_do[1], ch2_do_state);
        if (g_pdo_el2004_do[2]) ecat_wr_bit(g_pdo_el2004_do[2], ch3_do_state);
        if (g_pdo_el2004_do[3]) ecat_wr_bit(g_pdo_el2004_do[3], ch4_do_state);

        // Step 4: Write EL4008 Analog Outputs (Dynamic DAC voltage scaling)
        uint16_t max_voltage_raw = V_to_raw(target_ao_voltage);
        for (int ch = 0; ch < 8; ch++) {
            if (g_pdo_el4008_ao[ch]) {
                ecat_wr_u16(g_pdo_el4008_ao[ch], max_voltage_raw);
            }
        }

        // Dynamically compute the exact output voltage produced by the channel
        float actual_produced_voltage = (float)max_voltage_raw / 3276.7f;

        // Step 5: Write EL5072 SM2 Outputs
        if (g_pdo_el5072_cmd) ecat_wr_bit(g_pdo_el5072_cmd, false);
        if (g_pdo_el5072_val) ecat_wr_s32(g_pdo_el5072_val, 0);

        // Step 6: Send Ethernet frames
        ecat_send(g_master);

        // Telemetry Printout every 100 ms (DYNAMICALLY PRINT PRODUCED VOLTAGE & DIGITAL STATES)
        if (cycle_count % 100 == 0) {
            printf("Cyclic time (ms): %7.1f | Cycle count: %8u | LVDT Value: %9d (%6.3f mm) | Analog output value: %5.2f V | Encoder value: %5u | Digital output Value: CH1=%s CH2=%s CH3=%s CH4=%s\n",
                   1.0f,
                   cycle_count,
                   raw_lvdt,
                   displacement_mm,
                   actual_produced_voltage,
                   encoder_counts,
                   ch1_do_state ? "ON " : "OFF",
                   ch2_do_state ? "ON " : "OFF",
                   ch3_do_state ? "ON " : "OFF",
                   ch4_do_state ? "ON " : "OFF");
        }

        usleep(1000); // 1 ms cycle delay
        cycle_count++;
    }

    printf("[CLEANUP] Application exiting. Releasing EtherCAT Master...\n");
    ecat_rel_master(g_master);
}
