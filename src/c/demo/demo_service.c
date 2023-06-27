#include <stdio.h>
#include <stdlib.h>

#include "alg_sdk/service.h"

int load_board_fw_update_bin(const char *filename,uint8_t* payload, uint32_t *len)
{
    char *path = filename;
    FILE *fp = fopen(path, "r");
    printf("current path %s\r\n", path);
    if(fp == NULL)
    {
        printf("file open failed,maybe file name is error\r\n");
        return -1;
    }

    char *fw = malloc(1 * 1024 * 1024);
    if(fw == NULL)
    {
        printf("malloc error\r\n");
        fclose(fp);
        return -1;
    }

    uint32_t size = fread(fw, 1, 1 * 1024 * 1024, fp);
    if (size >= 1024)
    {
        printf("read fw bin size %d\r\n", size);
    }
    else
    {
        printf("file name is not right\r\n");
        return -1;
    }
    //copy fw update bin
    memcpy(payload,fw,size);
    *len = size;

    printf("size:%d\r\n",size);

    free(fw);
    fclose(fp);

    return 0;
}

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

            if (c == '\r')
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
            timeout = 10000;
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
    else if ((argc == 4) && (strcmp(argv[1], "-fw_update_by_file") == 0))
    {
        char *file_name = argv[2];
        const char *topic_name = "/service/board/fw_update";
        {
            service_board_fw_update_t t = {
                .ack_mode = 1,
                .board_id = 0,
            };
            t.board_id = atoi(argv[2]);
            FILE *fp = fopen(file_name, "r");
            printf("current path %s\r\n", file_name);
            if(fp == NULL)
            {
                printf("file open failed,maybe file name is error\r\n");
                return -1;
            }

            char *fw = malloc(64 * 1024 * 1024);
            if(fw == NULL)
            {
                printf("malloc error\r\n");
                fclose(fp);
                return -1;
            }
            uint32_t offset = 0;

            fseek(fp,0+offset,SEEK_SET);
            uint32_t size = fread(fw, 1, 64 * 1024 * 1024, fp);
            if (size >= 1024)
            {
                printf("read fw bin size %d\r\n", size);
            }
            else
            {
                printf("file name is not right\r\n");
                return -1;
            }

            printf("size:%d\r\n",size);
            uint8_t send_completed = 0;
            while (!send_completed)
            {
                if (size >= ALG_SDK_MAX_BIN_SIZE)
                {
                    memcpy(t.payload,fw+offset,ALG_SDK_MAX_BIN_SIZE);
                    t.fw_bin_size = ALG_SDK_MAX_BIN_SIZE;
                    size -= ALG_SDK_MAX_BIN_SIZE;
                    offset += ALG_SDK_MAX_BIN_SIZE;
                }
                else
                {
                    memcpy(t.payload,fw+offset,size);
                    t.fw_bin_size = size;
                    send_completed = 1;
                    printf("send finished!\r\n");
                }

                rc = alg_sdk_call_service(topic_name, &t, timeout);
                if (rc < 0)
                {
                    printf("Request Service : [%s] Error!\n", topic_name);
                    return -1;
                }

                printf("[ack : %d]\n", t.ack_code);
            }

            free(fw);
            fclose(fp);

        }

        return 0;

    }
    else if ((argc == 4) && (strcmp(argv[1], "-channel_info") == 0))
    {
        const char *topic_name = "/service/channel/info";
        service_board_channel_info_get_t  t = {
            .ack_mode = 1,
            .board_id = 0,
            .channel_num = 0,
        };
        t.board_id = atoi(argv[2]);
        t.channel_num = atoi(argv[3]);
        rc = alg_sdk_call_service(topic_name, &t, timeout);
        if (rc < 0)
        {
            printf( "Request Service : [%s] Error!\n", topic_name);
            return 0;
        }

        printf("[ack : %d]\n", t.ack_code);
        printf("camera_link_status: %d,camera_streaming_on_status: %d\r\n",t.camera_link_status,t.camera_streaming_on_status);
        printf("camera_name: %s\r\n",t.camera_name);
        printf("camera_width: %d,camera_height: %d\r\n",t.camera_width,t.camera_height);
        printf("camera_slv_trigger_start: %d\r\n",t.camera_slv_trigger_start);
    }
    else if ((argc == 4) && (strcmp(argv[1], "-get_i2c_addr_info") == 0))
    {
        const char *topic_name = "/service/i2c/info";
        service_board_i2c_info_get_t  t = {
            .ack_mode = 1,
            .board_id = 0,
            .channel_num = 0,
        };
        t.board_id = atoi(argv[2]);
        t.channel_num = atoi(argv[3]);
        rc = alg_sdk_call_service(topic_name, &t, timeout);
        if (rc < 0)
        {
            printf( "Request Service : [%s] Error!\n", topic_name);
            return 0;
        }

        printf("[ack : %d]\n", t.ack_code);
        printf("i2c_addr_dev_cnt: %d\r\n",t.i2c_addr_dev_cnt);
        for (int i = 0; i < t.i2c_addr_dev_cnt; i++)
        {
            printf("i2c_addr_array[%d]:0x%x\r\n",i,t.i2c_addr_array[i]);
        }
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
    else if ((argc == 3) && (strcmp(argv[1], "-get_board_info") == 0))
    {

        /* Example : Read IIC */
        const char *topic_name = "/service/board/get_info";

        service_board_info_get_t t = {
            .ack_mode = 1,
            .board_id = 0,
        };

        t.board_id = atoi(argv[2]);
        rc = alg_sdk_call_service(topic_name, &t, timeout);
        if (rc < 0)
        {
            printf( "Request Service : [%s] Error!\n", topic_name);
            return 0;
        }

        printf("[ack : %d]\n", t.ack_code);
        char *device_type = t.device_type;
        printf("device_type: %s\r\n",device_type);
        char *sn = t.SN;
        printf("SN: %s\r\n",sn);
        printf("hardware_version_num: %d\r\n",t.hardware_version_num);
        char *ps_ver = t.ps_firmware_version;
        printf("ps_ver: %s\r\n",ps_ver);
        char *pl_ver = t.pl_firmware_version;
        printf("pl_firmware_version: %s\r\n",pl_ver);
        printf("board_channel_num: %d\r\n",t.board_channel_num);
        /* End */
    }
    else if ((argc == 4) && (strcmp(argv[1], "-get_camera_info") == 0))
    {

        /* Example : Read IIC */
        const char *topic_name = "/service/camera/get_info";

        service_camera_info_get_t t = {
            .ack_mode = 1,
            .board_id = 0,
            .channel = 0,
        };
        t.board_id = atoi(argv[2]);
        t.channel = atoi(argv[3]);
        rc = alg_sdk_call_service(topic_name, &t, timeout);
        if (rc < 0)
        {
            printf( "Request Service : [%s] Error!\n", topic_name);
            return 0;
        }

        printf("[ack : %d]\n", t.ack_code);
        printf("[sensor_uid : 0x%x]\n", t.camera_info.sensor_uid);
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
