#include <stdio.h>
#include <stdlib.h>

#include "alg_sdk/service.h"

int load_sensor_config(const char* filename, uint8_t* payload, uint16_t* len)
{
    int len_line=0;
    FILE *fp = fopen(filename, "r");
    if (fp != NULL)
    {
        int buf[65535];
        int count = 0;
        int c;

        c = fgetc(fp);

        while (c != EOF && count < 65534)
        {
            c = fgetc(fp);
            fscanf(fp, "%x", buf + count);
            count++;

            if (c == '\n')
                len_line++;
            // printf("%x|",c);
        }
        len_line++;
        fclose(fp);

        if(payload)
        {
            for (int i = 0; i < len_line; i++)
            {
                payload[7 * i] = buf[i * 4];
                payload[7 * i + 1] = (buf[4 * i + 1] & 0xFF);
                payload[7 * i + 2] = (buf[4 * i + 1] >> 8);
                payload[7 * i + 3] = (buf[4 * i + 2] & 0xFF);
                payload[7 * i + 4] = (buf[4 * i + 2] >> 8);
                payload[7 * i + 5] = (buf[4 * i + 3] & 0xFF);
                payload[7 * i + 6] = (buf[4 * i + 3] >> 8);
            }
            *len = len_line;
            return 0;
        }
        else
        {
            printf("Failed to load sensor config : payload is NULL\n");
            return -1;
        }

    }
    else
    {
        printf("Failed to load sensor config %s\n", filename);
        return -1;
    }
}

int main (int argc, char **argv)
{
    int rc;
    int timeout = 5000;

    if ((argc == 2) && (strcmp(argv[1], "-set_config") == 0))
    {
        const char *topic_name = "/service/camera/set_config";

        // for (int j = 0; j < 2; j++)
        {
            service_camera_config_t t = {
                .ack_mode = 1,
                .module_type = 12,
            };
            t.ch_id = 0;

            printf("ch %d, type %d\n", t.ch_id, t.module_type);
            rc = alg_sdk_call_service(topic_name, &t, timeout);
            if (rc < 0)
            {
                printf("Request Service : [%s] Error!\n", topic_name);
                return 0;
            }

            printf("[ack : %d], [channel : %d]\n", t.ack_code, t.channel);
        }

        return 0;

    }

    else if ((argc == 3) && (strcmp(argv[1], "-set_config_by_file") == 0))
    {
        char *file_name = argv[2];
        const char *topic_name = "/service/camera/set_config";

        // for (int j = 0; j < 2; j++)
        {
            service_camera_config_t t = {
                .ack_mode = 1,
                .module_type = 0xFFFF, // 0xFFFF means Not ALG Camera
                .width = 3840,
                .height = 2160,
                .deser_mode = ALG_SDK_MAX_GMSL_6G_MODE,
            };
            t.ch_id = 0;

            int ret = load_sensor_config(file_name, &t.payload[0], &(t.line_len));

            if(ret < 0)
            {
                return 1;
            }

            /* Debug Info */
            // printf("line len %d\n", t.line_len);
            // for(int i=0;i<t.line_len;i++)
            // {
            //     printf("0x%x|0x%x|0x%x|0x%x|0x%x|0x%x|0x%x",
            //      t.payload[i*7], t.payload[i*7+1], t.payload[i*7+2],
            //      t.payload[i*7+3], t.payload[i*7+4], t.payload[i*7+5],
            //      t.payload[i*7+6]);

            //     printf("\n");
            // }
            /* Debug Info End */

            printf("ch %d, type %d\n", t.ch_id, t.module_type);
            rc = alg_sdk_call_service(topic_name, &t, timeout);
            if (rc < 0)
            {
                printf("Request Service : [%s] Error!\n", topic_name);
                return 0;
            }

            printf("[ack : %d], [channel : %d]\n", t.ack_code, t.channel);
        }

        return 0;

    }
    else if ((argc == 2) && (strcmp(argv[1], "-read_reg") == 0))
    {

        /* Example : Read IIC */
        const char *topic_name = "/service/camera/read_reg";

        service_camera_read_reg_t t1 = {
            .ack_mode = 1,
            .ch_id = 3,
            .msg_type = 0x1608,
            .device_addr = 0x90,
            .line_len = 1,
        };

        uint16_t reg[10] = {0x0000, 0xa291, 0xb102, 0xb2c3, 0xc194, 0xc2d8, 0xd296, 0xd267, 0xe598, 0xe620};

        for (int i = 0; i < t1.line_len; i++)
        {
            t1.payload[i] = reg[i];
        }

        rc = alg_sdk_call_service(topic_name, &t1, timeout);
        if (rc < 0)
        {
            printf( "Request Service : [%s] Error!\n", topic_name);
            return 0;
        }

        printf("[ack : %d], [channel : %d]\n", t1.ack_code, t1.channel);

        for (int i = 0; i < t1.length_r; i++)
        {
            printf("[len : %d][data : %d]\n", t1.length_r, t1.data[i]);
        }
        /* End */

    }
    else if ((argc == 2) && (strcmp(argv[1], "-stream_on") == 0))
    {
        /* Example : Stream On/Off */
        const char *topic_name = "/service/camera/stream_on";

        service_stream_control_t t1 = {
            .ack_mode = 1,
        };

        uint8_t sel[32] = {0};
        uint8_t ctl[32] = {0};

        sel[0] = 1;
        ctl[0] = 1;
        sel[2] = 1;
        ctl[2] = 0;
        sel[4] = 1;
        ctl[4] = 0;
        sel[6] = 1;
        ctl[6] = 0;

        for (int i = 0; i < ALG_SDK_MAX_CHANNEL; i++)
        {
            t1.select[i] = sel[i];
            t1.control[i] = ctl[i];
        }

        rc = alg_sdk_call_service(topic_name, &t1, timeout);
        if (rc < 0)
        {
            printf("Request Service : [%s] Error!\n", topic_name);
            return 0;
        }

        printf("[ack : %d], Channel SEL : \n", t1.ack_code);
        for (int i = 0; i < ALG_SDK_MAX_CHANNEL; i++)
        {
            printf("%d|", t1.ch_sel[i]);
        }
        printf("\n");
        /* End */
    }
    else if ((argc == 2) && (strcmp(argv[1], "-stream_off") == 0))
    {
        /* Example : Stream On/Off */
        const char *topic_name = "/service/camera/stream_on";

        service_stream_control_t t1 = {
            .ack_mode = 1,
        };

        uint8_t sel[32] = {0};
        uint8_t ctl[32] = {0};

        sel[0] = 1;
        ctl[0] = 0;
        sel[2] = 1;
        ctl[2] = 0;
        sel[4] = 1;
        ctl[4] = 0;
        sel[6] = 1;
        ctl[6] = 0;

        for (int i = 0; i < ALG_SDK_MAX_CHANNEL; i++)
        {
            t1.select[i] = sel[i];
            t1.control[i] = ctl[i];
        }

        rc = alg_sdk_call_service(topic_name, &t1, timeout);
        if (rc < 0)
        {
            printf("Request Service : [%s] Error!\n", topic_name);
            return 0;
        }

        printf("[ack : %d], Channel SEL : \n", t1.ack_code);
        for (int i = 0; i < ALG_SDK_MAX_CHANNEL; i++)
        {
            printf("%d|", t1.ch_sel[i]);
        }
        printf("\n");
        /* End */
    }
    return 0;
}
