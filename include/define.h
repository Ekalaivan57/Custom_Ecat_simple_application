#ifndef DEFINE_H
#define DEFINE_H

#include <stdint.h>
#include <stdbool.h>

/* Vendor ID for Beckhoff Automation GmbH */
#define BECKHOFF_VENDOR_ID  0x00000002

/* Position 0: EK1100 Bus Coupler */
#define EK1100_ALIAS        0
#define EK1100_POS          0
#define EK1100_PRODUCT      0x044c2c52

/* Position 1: EL5101 1-Channel 5V Incremental Encoder */
#define EL5101_ALIAS        0
#define EL5101_POS          1
#define EL5101_PRODUCT      0x13ed3052

/* Position 2: EL5072 2-Channel LVDT / Inductive Sensor Interface */
#define EL5072_ALIAS        0
#define EL5072_POS          2
#define EL5072_PRODUCT      0x13d03052

/* Position 3: EL4008 8-Channel 0-10V Analog Output */
#define EL4008_ALIAS        0
#define EL4008_POS          3
#define EL4008_PRODUCT      0x0fa83052

/* Position 4: EL2004 4-Channel 24V Digital Output */
#define EL2004_ALIAS        0
#define EL2004_POS          4
#define EL2004_PRODUCT      0x07d43052

/* Calibration & Scaling Constants */
#define LVDT_GAIN_SCALE     4.680e-6
#define LVDT_HOMING_OFFSET  (-320810.5)

/* Voltage conversion helper: converts 0.0V to 10.0V -> 0 to 32767 raw DAC counts */
static inline uint16_t V_to_raw(float volts) {
    if (volts > 10.0f) volts = 10.0f;
    if (volts < 0.0f)  volts = 0.0f;
    return (uint16_t)(volts * 3276.7f);
}

#endif /* DEFINE_H */
