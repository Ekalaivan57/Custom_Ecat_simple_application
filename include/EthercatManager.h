/****************=============================================================
 *
 * ERL Spectra EtherCAT Custom Application - EthercatManager Header
 * File: EthercatManager.h
 *
 * Provides API mapping for Hydrive Platform Firmware using ecat_api.h
 *
 *=============================================================================*/

#ifndef ETHERCAT_MANAGER_H
#define ETHERCAT_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************=============================================================
 * CORE LIFECYCLE & REAL-TIME RUNNER APIS
 * ==========================================================================*/

/** @brief Initializes EtherCAT master, configures slaves, and registers PDOs. */
int init_ethercat(void);

/** @brief Executes single real-time cyclic step. */
bool Ethercat_Run(void);

/** @brief Safely shuts down outputs and releases master. */
void Ethercat_Cleanup(void);

/** @brief Explicitly receive Ethernet frame and process domain data. */
void recv_domain(void);

/** @brief Explicitly queue domain data and send Ethernet frame. */
void send_domain(void);

/****************=============================================================
 * SENSOR READERS & SCALING APIS
 * ==========================================================================*/

/** @brief Reads raw Encoder 16-bit counter value. */
uint16_t get_encoder_val(void);

/** @brief Calculates LVDT position displacement in mm. */
float getLVDTpos(void);

/** @brief Reads calibrated LVDT position integer value. */
int getLVDT(void);

/** @brief Reads Load Cell / Strain Gauge force value in kN. */
float getLoad(void);

/** @brief Reads raw Load Cell value. */
float getLoadRaw(void);

/** @brief Reads System Oil Temperature in degrees Celsius. */
int16_t gettemperature(void);

/** @brief Connection check for LVDT sensor. */
bool Lvdt_Check(void);

/** @brief Connection check for Load Cell sensor. */
bool Lc_Check(void);

/****************=============================================================
 * ACTUATOR & OUTPUT CONTROLLERS
 * ==========================================================================*/

/** @brief Sends raw DAC angle/position signal to Analog Output. */
void SendPosition(int16_t angle);

/** @brief Sets output voltage (0.0V to 10.0V) on EL4008 channel (0..7). */
void set_analog_output_volts(uint8_t channel, double volts);

/** @brief Sets Digital Output state (ON/OFF) on EL2004 channel (0..3). */
void set_digital_output(uint8_t channel, bool state);

/****************=============================================================
 * ZEROING & TARE CALIBRATION APIS
 * ==========================================================================*/

/** @brief Calibrates and stores LVDT zero homing offset. */
void LVDT_Zero_Set(void);

/** @brief Zeroes Load Cell Tare. */
void LOADCELL_Zero_set(void);

/** @brief Executes EL3356 Tare calibration procedure. */
void El3356_tare(void);

#ifdef __cplusplus
}
#endif

#endif /* ETHERCAT_MANAGER_H */
