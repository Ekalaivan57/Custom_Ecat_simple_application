/****************=============================================================
 *
 * ERL Spectra EtherCAT Custom Application - Macros & Hardware Definitions
 * File: define.h
 *
 *=============================================================================*/

#ifndef DEFINE_H
#define DEFINE_H

#include <stdint.h>
#include <stdbool.h>

/****************=============================================================
 * VENDOR & PRODUCT CODES
 * ==========================================================================*/

#define BECKHOFF_VENDOR_ID          0x00000002

#define PRODUCT_EK1100              0x044c2c52  /* Coupler */
#define PRODUCT_EL5101              0x13ed3052  /* Incremental Encoder */
#define PRODUCT_EL5072              0x13d03052  /* LVDT Interface */
#define PRODUCT_EL4008              0x0fa83052  /* 8Ch Analog Output 0-10V */
#define PRODUCT_EL2004              0x07d43052  /* 4Ch Digital Output 24V */

/****************=============================================================
 * SLAVE POSITIONS ON THE ETHERCAT BUS
 * ==========================================================================*/

#define POS_EK1100                  0
#define POS_EL5101                  1
#define POS_EL5072                  2
#define POS_EL4008                  3
#define POS_EL2004                  4

/****************=============================================================
 * PDO INDEX & SUBINDEX DEFINITIONS
 * ==========================================================================*/

/* Slave 1: EL5101 Encoder Inputs */
#define PDO_EL5101_STATUS_INDEX     0x6000
#define PDO_EL5101_STATUS_SUB       0x01
#define PDO_EL5101_VALUE_INDEX      0x6000
#define PDO_EL5101_VALUE_SUB       0x02

/* Slave 2: EL5072 LVDT Outputs (Required for OP State) & Inputs */
#define PDO_EL5072_SET_CMD_INDEX    0x7000
#define PDO_EL5072_SET_CMD_SUB      0x01
#define PDO_EL5072_SET_VAL_INDEX    0x7000
#define PDO_EL5072_SET_VAL_SUB      0x11
#define PDO_EL5072_POS_CH1_INDEX    0x6001
#define PDO_EL5072_POS_CH1_SUB      0x01

/* Slave 3: EL4008 8-Channel Analog Output (16-bit each, 0 to 10V) */
#define PDO_EL4008_AO1_INDEX        0x7000
#define PDO_EL4008_AO2_INDEX        0x7010
#define PDO_EL4008_AO3_INDEX        0x7020
#define PDO_EL4008_AO4_INDEX        0x7030
#define PDO_EL4008_AO5_INDEX        0x7040
#define PDO_EL4008_AO6_INDEX        0x7050
#define PDO_EL4008_AO7_INDEX        0x7060
#define PDO_EL4008_AO8_INDEX        0x7070
#define PDO_EL4008_AO_SUB           0x01

/* Slave 4: EL2004 4-Channel Digital Output (1-bit each) */
#define PDO_EL2004_DO1_INDEX        0x7000
#define PDO_EL2004_DO2_INDEX        0x7010
#define PDO_EL2004_DO3_INDEX        0x7020
#define PDO_EL2004_DO4_INDEX        0x7030
#define PDO_EL2004_DO_SUB           0x01

/****************=============================================================
 * APPLICATION SCALING & CALIBRATION CONSTANTS
 * ==========================================================================*/

#define LVDT_GAIN_SCALE             4.680e-6    /* Gain multiplier */
#define LVDT_HOMING_OFFSET_DEFAULT  (-320810.5) /* Default homing zero offset */

#define DAC_COUNTS_PER_VOLT         3276.7      /* 32767 counts / 10.0 V */
#define ANALOG_MAX_VOLTS            10.0        /* Max 10.0 V */
#define CYCLE_PERIOD_US             1000        /* 1 ms Real-Time Cycle */

/****************=============================================================
 * HELPER INLINE CONVERSION FUNCTIONS
 * ==========================================================================*/

/**
 * @brief Converts voltage (-10.0V to +10.0V) to raw 16-bit DAC counts.
 */
static inline int16_t V_to_raw(double volts) {
    if (volts > 10.0)  volts = 10.0;
    if (volts < -10.0) volts = -10.0;
    return (int16_t)(volts * DAC_COUNTS_PER_VOLT);
}

/**
 * @brief Converts raw 16-bit DAC counts to voltage (-10.0V to +10.0V).
 */
static inline double raw_to_V(int16_t raw) {
    return (double)raw / DAC_COUNTS_PER_VOLT;
}

#endif /* DEFINE_H */
