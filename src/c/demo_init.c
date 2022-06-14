#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "alg_sdk/alg_sdk.h"

void int_handler(int sig)
{
    // printf("Caught signal : %d\n", sig);
    alg_sdk_stop();

    /* terminate program */
    exit(sig);
}

int main (int argc, char **argv)
{
    if ((argc == 2) && (strcmp (argv[1], "-s") == 0))
    {
        signal(SIGINT, int_handler);
        int rc;
        int frame_rate = 1000;
        rc = alg_sdk_init(frame_rate);
        if(rc < 0)
        {
            printf("Init SDK Failed\n");
            exit(0);
        }

        // alg_sdk_stop();
        alg_sdk_spin_on();
    }

    return 0;
}
