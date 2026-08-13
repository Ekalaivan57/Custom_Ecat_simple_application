#ifndef ETHERCAT_MANAGER_H
#define ETHERCAT_MANAGER_H

#include <stdbool.h>
#include <stdint.h>
#include <ecat_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize EtherCAT Master, Domain, Slaves, and PDO Registration */
bool init_ethercat(void);

/* Set LVDT Zero Homing Offset */
void LVDT_Zero_Set(int val);

/* Read Calibrated LVDT Position in Millimeters */
float getLVDTpos(void);

/* Run Real-Time Cyclic Execution Loop */
void Ethercat_Run(bool *running);

#ifdef __cplusplus
}
#endif

#endif /* ETHERCAT_MANAGER_H */
