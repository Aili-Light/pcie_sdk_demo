#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pub_sub/alg_sdk.h"

int main (int argc, char **argv)
{
    if ((argc == 2) && (strcmp (argv[1], "-s") == 0))
    {
        int rc;
        int frame_rate = 30;
        rc = alg_sdk_init(frame_rate);
        if(rc < 0)
        {
            printf("Init SDK Failed\n");
            exit(0);
        }

        alg_sdk_stop();
    }

    return 0;
}
