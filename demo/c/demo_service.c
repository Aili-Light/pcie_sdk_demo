#include <stdio.h>
#include <stdlib.h>

#include "pub_sub/service.h"


int main (int argc, char **argv)
{
    if ((argc == 2) && (strcmp(argv[1], "-p") == 0))
    {
        int rc;
        int timeout = 5000;

        if (0)
        {
            const char *topic_name = "/service/camera/set_config";

            // service_camera_config_t t = {
            //     .ack_mode = 0,
            //     .ch_id = 2,
            //     .module_type = 0xFFFF, // ISX021
            //     .width = 1280,
            //     .height = 960,
            //     .length = 2,
            // };
            // // t.payload = malloc(sizeof(uint8_t) * 7*t.length);

            // uint8_t addr[2] = {0x01, 0x02};
            // uint16_t reg[2] = {0x03e8, 0x07d0};
            // uint16_t data[2] = {0x0fa0, 0x1388};
            // uint16_t fmt[2] = {0x1770, 0x1b58};

            // for(int i=0;i<2;i++)
            // {
            //     t.payload[7*i] = addr[i];
            //     t.payload[7*i+1] = reg[i] & 0xFF;
            //     t.payload[7*i+2] = reg[i] >> 8;
            //     t.payload[7*i+3] = data[i] & 0xFF;
            //     t.payload[7*i+4] = data[i] >> 8;
            //     t.payload[7*i+5] = fmt[i] & 0xFF;
            //     t.payload[7*i+6] = fmt[i] >> 8;
            // }
            // printf("%d|%d|%d\n", t.payload[0], t.payload[1], t.payload[3]);

            for (int i = 0; i < 8; i++)
            {
                service_camera_config_t t = {
                    .ack_mode = 1,
                    .ch_id = 0,
                    .module_type = ALG_SDK_SENSOR_TYPE_OV_OX8B, // ISX021
                };
                t.ch_id = i;
                // printf("ch %d, type %d\n", t.ch_id, t.module_type);
                rc = alg_sdk_call_service(topic_name, &t, timeout);
                if (rc < 0)
                {
                    alg_sdk_log(LOG_ERROR, "Request Service : [%s] Error!\n", topic_name);
                    return 0;
                }

                printf("[ack : %d], [channel : %d]\n", t.ack_code, t.channel);
            }
        }
        if (0)
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
                alg_sdk_log(LOG_ERROR, "Request Service : [%s] Error!\n", topic_name);
                return 0;
            }

            printf("[ack : %d], [channel : %d]\n", t1.ack_code, t1.channel);

            for (int i = 0; i < t1.length_r; i++)
            {
                printf("[len : %d][data : %d]\n", t1.length_r, t1.data[i]);
            }
            /* End */
        }
        if (1)
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
                alg_sdk_log(LOG_ERROR, "Request Service : [%s] Error!\n", topic_name);
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
    }

    return 0;
}