#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "alg_sdk/alg_sdk.h"
#ifdef __cplusplus
extern "C" {
#endif


void int_handler(int sig)
{
    // printf("Caught signal : %d\n", sig);
    alg_sdk_stop();

    /* terminate program */
    exit(sig);
}

int main (int argc, char **argv)
{
    signal(SIGINT, int_handler);

    if ((argc == 2) && (strcmp (argv[1], "-s") == 0))
    {
        int rc;
        int frq = 1000;
        rc = alg_sdk_init(frq);
        if(rc < 0)
        {
            printf("Init SDK Failed\n");
            exit(0);
        }

        alg_sdk_spin_on();
    }
    else if ((argc == 2) && (strcmp (argv[1], "-v") == 0))
    {
#if defined (WITH_STREAM)
        int rc;
        char *appsrc[] = {"--stream", "protocol=v4l2", "v4l2_device=/dev/video0", "codec_type=raw"};
        rc = alg_sdk_init_v2(4, &appsrc[0]);
        if(rc < 0)
        {
            printf("Init SDK Failed\n");
            exit(0);
        }
        
        alg_sdk_spin_on();
#else
        printf("Please add option -DWITH_GSTREAM=ON for cmake!\n");
        exit(0);
#endif
    }
    else if ((argc == 2) && (strcmp (argv[1], "-r") == 0))
    {
#if defined (WITH_STREAM) && defined (WITH_CUDA)
        int rc;
        char *appsrc[] = {"--stream", "protocol=record", "codec_type=h265", "file_location=."};
        rc = alg_sdk_init_v2(4, &appsrc[0]);
        if(rc < 0)
        {
            printf("Init SDK Failed\n");
            exit(0);
        }

        alg_sdk_spin_on();
#else
        printf("Please add option -DWITH_GSTREAM=ON and -DWITH_CUDA=ON for cmake!\n");
        exit(0);
#endif
    }
    else if ((argc == 2) && (strcmp (argv[1], "-d") == 0))
    {
#if defined (WITH_STREAM) && defined (WITH_CUDA)
        int rc;
        char *appsrc[] = {"--stream", "protocol=display", "codec_type=raw"};
        rc = alg_sdk_init_v2(3, &appsrc[0]);
        if(rc < 0)
        {
            printf("Init SDK Failed\n");
            exit(0);
        }

        alg_sdk_spin_on();
#else
        printf("Please add option -DWITH_GSTREAM=ON and -DWITH_CUDA=ON for cmake!\n");
        exit(0);
#endif
    }
    else if ((argc == 2) && (strcmp (argv[1], "-w") == 0))
    {
#if defined (WITH_STREAM) && defined (WITH_CUDA)
        int rc;
        char *appsrc[] = {"--stream", "protocol=rtp", "host_ip=127.0.0.1", "port_num=5000", "codec_type=h264"};
        rc = alg_sdk_init_v2(5, &appsrc[0]);
        if(rc < 0)
        {
            printf("Init SDK Failed\n");
            exit(0);
        }

        alg_sdk_spin_on();
#else
        printf("Please add option -DWITH_GSTREAM=ON and -DWITH_CUDA=ON for cmake!\n");
        exit(0);
#endif
    }
    else
    {
        int rc;
        rc = alg_sdk_init_v2(argc, argv);
        if(rc < 0)
        {
            printf("Init SDK Failed\n");
            exit(0);
        }

        alg_sdk_spin_on();
    }
    return 0;
}
#ifdef __cplusplus
}
#endif
