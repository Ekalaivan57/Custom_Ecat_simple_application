/****************=============================================================
 *
 * ERL Spectra EtherCAT Custom Application - Main Entry Point
 * File: main.c
 *
 *=============================================================================*/

#include <stdio.h>
#include <signal.h>
#include "EthercatManager.h"

static volatile bool g_running = true;

static void signal_handler(int sig)
{
    (void)sig;
    g_running = false;
}

int main(void)
{
    /* Trap system signals for graceful exit */
    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGTSTP, signal_handler);

    /* Initialize EtherCAT Master, Slaves, and PDOs */
    if (init_ethercat() < 0) {
        fprintf(stderr, "Application Initialization Failed.\n");
        return -1;
    }

    /* Real-Time Cyclic Execution Loop */
    while (g_running) {
        if (!Ethercat_Run()) {
            break;
        }
    }

    /* Clean Shutdown */
    Ethercat_Cleanup();
    return 0;
}
