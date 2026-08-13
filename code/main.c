#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include "EthercatManager.h"

static bool g_running = true;

void signal_handler(int sig)
{
    (void)sig;
    g_running = false;
}

int main(void)
{
    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);

    if (!init_ethercat()) {
        fprintf(stderr, "[ERROR] EtherCAT Application Initialization Failed!\n");
        return -1;
    }

    printf(">>> Running Real-Time Application Loop (Press Ctrl+C to stop) <<<\n\n");
    Ethercat_Run(&g_running);

    return 0;
}
