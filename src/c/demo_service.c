#include <stdio.h>
#include <stdlib.h>

#include "alg_sdk/service.h"


int main (int argc, char **argv)
{
    if ((argc == 2) && (strcmp(argv[1], "-p") == 0))
    {
        int rc;
        int timeout = 5000;

        if (1)
        {
            const char *topic_name = "/service/camera/set_config";

            // service_camera_config_t t = {
            //     .ack_mode = 0,
            //     .ch_id = 2,
            //     .module_type = 0xFFFF, // ISX021
            //     .width = 3840,
            //     .height = 2160,
            //     .line_len = 21,
            // };
            // // t.payload = malloc(sizeof(uint8_t) * 7*t.length);

            // uint8_t addr[21] = {0x00,
            //     0xc0,
            //     0xc0,
            //     0xc0,
            //     0xc0,
            //     0xc0,
            //     0x00,
            //     0x90,
            //     0x90,
            //     0x90,
            //     0x90,
            //     0x90,
            //     0x90,
            //     0x00,
            //     0xc0,
            //     0xc0,
            //     0xc0,
            //     0xc0,
            //     0xc0,
            //     0x00,
            //     0xc0};
            // uint16_t reg[21] = {0x0000,
            //     0x02d6,
            //     0x02c1,
            //     0x02be,
            //     0x03f1,
            //     0x03f0,
            //     0x0000,
            //     0x0330,
            //     0x0002,
            //     0x0051,
            //     0x0052,
            //     0x044a,
            //     0x0320,
            //     0x0000,
            //     0x0383,
            //     0x0318,
            //     0x0570,
            //     0x02c1,
            //     0x02be,
            //     0x0000,
            //     0x02d6};
            // uint16_t data[21] = {0xff,
            //     0x80,
            //     0x80,
            //     0x90,
            //     0x09,
            //     0x59,
            //     0x0f,
            //     0x04,
            //     0xf3,
            //     0x02,
            //     0x01,
            //     0xd0,
            //     0x2c,
            //     0xff,
            //     0x00,
            //     0x5e,
            //     0x0c,
            //     0x90,
            //     0x90,
            //     0xff,
            //     0x90};
            // uint16_t fmt[21] = {0x0808
            //     0x1608,
            //     0x1608,
            //     0x1608,
            //     0x1608,
            //     0x1608,
            //     0x0808,
            //     0x1608,
            //     0x1608,
            //     0x1608,
            //     0x1608,
            //     0x1608,
            //     0x1608,
            //     0x0808,
            //     0x1608,
            //     0x1608,
            //     0x1608,
            //     0x1608,
            //     0x1608,
            //     0x0808,
            //     0x1608};

            // for(int i=0;i<21;i++)
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

            for (int i = 0; i < 4; i++)
            {
            service_camera_config_t t = {
                .ack_mode = 0,
                .ch_id = 0,
                .module_type = 0xFFFF, // ISX021
                .width = 3840,
                .height = 2160,
                .line_len = 21,
            };
            // t.payload = malloc(sizeof(uint8_t) * 7*t.length);

            uint8_t addr[21] = {0x00,
                0xc0,
                0xc0,
                0xc0,
                0xc0,
                0xc0,
                0x00,
                0x90,
                0x90,
                0x90,
                0x90,
                0x90,
                0x90,
                0x00,
                0xc0,
                0xc0,
                0xc0,
                0xc0,
                0xc0,
                0x00,
                0xc0};
            uint16_t reg[21] = {0x0000,
                0x02d6,
                0x02c1,
                0x02be,
                0x03f1,
                0x03f0,
                0x0000,
                0x0330,
                0x0002,
                0x0051,
                0x0052,
                0x044a,
                0x0320,
                0x0000,
                0x0383,
                0x0318,
                0x0570,
                0x02c1,
                0x02be,
                0x0000,
                0x02d6};
            uint16_t data[21] = {0xff,
                0x80,
                0x80,
                0x90,
                0x09,
                0x59,
                0x0f,
                0x04,
                0xf3,
                0x02,
                0x01,
                0xd0,
                0x2c,
                0xff,
                0x00,
                0x5e,
                0x0c,
                0x90,
                0x90,
                0xff,
                0x90};
            uint16_t fmt[21] = {0x0808,
                0x1608,
                0x1608,
                0x1608,
                0x1608,
                0x1608,
                0x0808,
                0x1608,
                0x1608,
                0x1608,
                0x1608,
                0x1608,
                0x1608,
                0x0808,
                0x1608,
                0x1608,
                0x1608,
                0x1608,
                0x1608,
                0x0808,
                0x1608};

            for(int j=0;j<21;j++)
            {
                t.payload[7*j] = addr[j];
                t.payload[7*j+1] = reg[j] & 0xFF;
                t.payload[7*j+2] = reg[j] >> 8;
                t.payload[7*j+3] = data[j] & 0xFF;
                t.payload[7*j+4] = data[j] >> 8;
                t.payload[7*j+5] = fmt[j] & 0xFF;
                t.payload[7*j+6] = fmt[j] >> 8;
            }
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
        if (0)
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
