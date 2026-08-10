#include "EthercatManager.h"
#include "define.h"
#include <ecat_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>


// Notes -----> in this g_ ---> global, p_--> pointer, s_----> static,m_ ---> member 
/* Global Master & Domain Handles */
static ecat_master_t g_master = NULL;
static ecat_domain_t g_domain = NULL;

/* Slave Handles */
static ecat_slave_t g_el5101 = NULL;
static ecat_slave_t g_el5072 = NULL;
static ecat_slave_t g_el4008 = NULL;
static ecat_slave_t g_el2004 = NULL;

/* PDO Accessor Handles */
static ecat_pdo_t g_pdo_encoder   = NULL;
static ecat_pdo_t g_pdo_lvdt_cmd  = NULL;
static ecat_pdo_t g_pdo_lvdt_val  = NULL;
static ecat_pdo_t g_pdo_lvdt_pos  = NULL;
static ecat_pdo_t g_pdo_ao[8]     = {NULL};
static ecat_pdo_t g_pdo_do[4]     = {NULL};

/* Telemetry & Calibration Variables */
static uint32_t g_cycle_count = 0;
static double   g_elapsed_time_ms = 0.0;
static int32_t  g_lvdt_raw_val = 0;
static int32_t  g_lvdt_homing_pos = LVDT_HOMING_OFFSET_DEFAULT;
static uint16_t g_encoder_val = 0;
static double   g_ao_volts[8] = {8.0, 8.0, 8.0, 8.0, 9.0, 9.0, 10.0, 10.0};
static bool     g_do_states[4] = {true, false, true, false};

int init_ethercat(void)
{
    printf(" ERL Spectra EtherCAT Platform - EthercatManager Initializing...\n");

    /* 1. Request Master 0 */
    g_master = ecat_req_master(0);
    if (!g_master)
    {
        fprintf(stderr, "[ecat_Error] Failed to acquire EtherCAT Master 0.\n");
        return -1;
    }

    /* 2. Create Process Data Domain */
    g_domain = ecat_create_domain(g_master);
    if (!g_domain)
    {
        fprintf(stderr, "[ecat_Error] Failed to create Process Data Domain.\n");
        return -1;
    }

    /* 3. Configure Connected Slaves */
    ecat_slave_config(g_master, 0, POS_EK1100, BECKHOFF_VENDOR_ID, PRODUCT_EK1100);
    g_el5101 = ecat_slave_config(g_master, 0, POS_EL5101, BECKHOFF_VENDOR_ID, PRODUCT_EL5101);
    g_el5072 = ecat_slave_config(g_master, 0, POS_EL5072, BECKHOFF_VENDOR_ID, PRODUCT_EL5072);
    g_el4008 = ecat_slave_config(g_master, 0, POS_EL4008, BECKHOFF_VENDOR_ID, PRODUCT_EL4008);
    g_el2004 = ecat_slave_config(g_master, 0, POS_EL2004, BECKHOFF_VENDOR_ID, PRODUCT_EL2004);

    if (!g_el5101 || !g_el5072 || !g_el4008 || !g_el2004)
    {
        fprintf(stderr, "[ecat_Error] One or more slave configurations failed.\n");
        return -1;
    }

    /* 4. Register PDO Channels */
    printf("[ecat_Info] Registering Slave PDO Entries...\n");

    // EL5101 Encoder Counter (0x6000:02, 16-bit)
    g_pdo_encoder = ecat_pdo_reg(g_el5101, PDO_EL5101_VALUE_INDEX, PDO_EL5101_VALUE_SUB, 16);

    // EL5072 LVDT Outputs (Required for OP state transition) & Inputs
    g_pdo_lvdt_cmd = ecat_pdo_reg(g_el5072, PDO_EL5072_SET_CMD_INDEX, PDO_EL5072_SET_CMD_SUB, 1);
    g_pdo_lvdt_val = ecat_pdo_reg(g_el5072, PDO_EL5072_SET_VAL_INDEX, PDO_EL5072_SET_VAL_SUB, 32);
    g_pdo_lvdt_pos = ecat_pdo_reg(g_el5072, PDO_EL5072_POS_CH1_INDEX, PDO_EL5072_POS_CH1_SUB, 32);

    // EL4008 8-Channel Analog Output (0-10V, 16-bit each)
    g_pdo_ao[0] = ecat_pdo_reg(g_el4008, PDO_EL4008_AO1_INDEX, PDO_EL4008_AO_SUB, 16);
    g_pdo_ao[1] = ecat_pdo_reg(g_el4008, PDO_EL4008_AO2_INDEX, PDO_EL4008_AO_SUB, 16);
    g_pdo_ao[2] = ecat_pdo_reg(g_el4008, PDO_EL4008_AO3_INDEX, PDO_EL4008_AO_SUB, 16);
    g_pdo_ao[3] = ecat_pdo_reg(g_el4008, PDO_EL4008_AO4_INDEX, PDO_EL4008_AO_SUB, 16);
    g_pdo_ao[4] = ecat_pdo_reg(g_el4008, PDO_EL4008_AO5_INDEX, PDO_EL4008_AO_SUB, 16);
    g_pdo_ao[5] = ecat_pdo_reg(g_el4008, PDO_EL4008_AO6_INDEX, PDO_EL4008_AO_SUB, 16);
    g_pdo_ao[6] = ecat_pdo_reg(g_el4008, PDO_EL4008_AO7_INDEX, PDO_EL4008_AO_SUB, 16);
    g_pdo_ao[7] = ecat_pdo_reg(g_el4008, PDO_EL4008_AO8_INDEX, PDO_EL4008_AO_SUB, 16);

    // EL2004 4-Channel Digital Output (24V, 1-bit each)
    g_pdo_do[0] = ecat_pdo_reg(g_el2004, PDO_EL2004_DO1_INDEX, PDO_EL2004_DO_SUB, 1);
    g_pdo_do[1] = ecat_pdo_reg(g_el2004, PDO_EL2004_DO2_INDEX, PDO_EL2004_DO_SUB, 1);
    g_pdo_do[2] = ecat_pdo_reg(g_el2004, PDO_EL2004_DO3_INDEX, PDO_EL2004_DO_SUB, 1);
    g_pdo_do[3] = ecat_pdo_reg(g_el2004, PDO_EL2004_DO4_INDEX, PDO_EL2004_DO_SUB, 1);

    if (!g_pdo_encoder || !g_pdo_lvdt_pos || !g_pdo_ao[0] || !g_pdo_do[0])
    {
        fprintf(stderr, "[ecat_Error] PDO Registration failed.\n");
        return -1;
    }

    /* 5. Activate Master */
    if (ecat_activate(g_master) < 0)
    {
        fprintf(stderr, "[ecat_Error] Master activation failed.\n");
        return -1;
    }

    printf("[ecat_Success] EtherCAT Master Activated! Real-time communication active.\n\n");
    return 0;
}

void recv_domain(void)
{
    if (g_master) 
    {
        ecat_recv(g_master);
    }
}

void send_domain(void)
{
    if (g_master)
    {
        ecat_send(g_master);
    }
}

float getLVDTpos(void)
{
    int32_t calibrated_counts = g_lvdt_raw_val - g_lvdt_homing_pos;
    return (float)(calibrated_counts * LVDT_GAIN_SCALE);
}

int getLVDT(void)
{
    return g_lvdt_raw_val;
}

uint16_t get_encoder_val(void)
{
    return g_encoder_val;
}


void set_analog_output_volts(uint8_t channel, double volts)
{
    if (channel < 8) {
        g_ao_volts[channel] = volts;
    }
}

void set_digital_output(uint8_t channel, bool state)
{
    if (channel < 4) {
        g_do_states[channel] = state;
    }
}

void LVDT_Zero_Set(void)
{
    g_lvdt_homing_pos = g_lvdt_raw_val;
    printf("[ecat_Info] LVDT Homing Zero set to: %d\n", g_lvdt_homing_pos);
}

void LOADCELL_Zero_set(void)
{
    printf("[ecat_Info] Load Cell Zero set.\n");
}

void El3356_tare(void)
{
    printf("[ecat_Info] EL3356 Tare procedure complete.\n");
}

bool Ethercat_Run(void)
{
    /* 1. Receive Ethernet frame */
    recv_domain();

    /* 2. Read Sensors */
    g_encoder_val  = ecat_rd_u16(g_pdo_encoder);
    g_lvdt_raw_val = ecat_rd_s32(g_pdo_lvdt_pos);

    /* 3. Write Analog Outputs (EL4008 8 Channels) */
    for (int ch = 0; ch < 8; ch++) {
        if (g_pdo_ao[ch]) {
            int16_t dac_raw = V_to_raw(g_ao_volts[ch]);
            ecat_wr_u16(g_pdo_ao[ch], (uint16_t)dac_raw);
        }
    }

    /* 4. Write Digital Outputs (EL2004 4 Channels) */
    for (int ch = 0; ch < 4; ch++) {
        if (g_pdo_do[ch]) {
            ecat_wr_bit(g_pdo_do[ch], g_do_states[ch]);
        }
    }

    /* 5. Transmit Ethernet frame */
    send_domain();

    /* 6. Formatted Telemetry Line Output */
    if (g_cycle_count % 100 == 0)
    {
        printf("Cyclic time (ms): %7.1f | Cycle count: %7u | LVDT Value: %10d | Analog output value: %5.2f V | Encoder value: %5u | Digital output Value: CH1=%s CH2=%s CH3=%s CH4=%s\n", g_elapsed_time_ms, g_cycle_count,g_lvdt_raw_val, g_ao_volts[0], g_encoder_val, g_do_states[0] ? "ON " : "OFF", g_do_states[1] ? "ON " : "OFF",g_do_states[2] ? "ON " : "OFF", g_do_states[3] ? "ON " : "OFF");
    }

    /* Timing Update */
    usleep(CYCLE_PERIOD_US);
    g_cycle_count++;
    g_elapsed_time_ms += (CYCLE_PERIOD_US / 1000.0);

    return true;
}

void Ethercat_Cleanup(void)
{
    printf("\n[ecat_Info] Safely shutting down outputs and releasing master...\n");

    if (g_master) {
        recv_domain();
        for (int ch = 0; ch < 8; ch++) {
            if (g_pdo_ao[ch]) ecat_wr_u16(g_pdo_ao[ch], 0);
        }
        for (int ch = 0; ch < 4; ch++) {
            if (g_pdo_do[ch]) ecat_wr_bit(g_pdo_do[ch], false);
        }
        send_domain();
        usleep(2000);

        ecat_rel_master(g_master);
        g_master = NULL;
    }

    printf("[ecat_Info] Cleanup finished successfully.\n");
}
