/*
 The MIT License (MIT)

Copyright (c) 2022 Aili-Light. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef __BASIC_TYPES_H__
#define __BASIC_TYPES_H__

#define ALG_SDK_SEM_INIT_BUFFER_SIZE 1024

#define ALG_SDK_IMAGE_QUEUE_MAX_LEN 32

#define ALG_SDK_IMAGE_WIDTH_ISX021_CONFIG_1 1920
#define ALG_SDK_IMAGE_HEIGHT_ISX021_CONFIG_1 1280

#define ALG_SDK_IMAGE_WIDTH_ISX019_CONFIG_1 1280
#define ALG_SDK_IMAGE_HEIGHT_ISX019_CONFIG_1 960

#define ALG_SDK_IMAGE_WIDTH_ISX031_CONFIG_1 1920
#define ALG_SDK_IMAGE_HEIGHT_ISX031_CONFIG_1 1536

#define ALG_SDK_IMAGE_WIDTH_OX03C_CONFIG_1 1920
#define ALG_SDK_IMAGE_HEIGHT_OX03C_CONFIG_1 1280

#define ALG_SDK_IMAGE_WIDTH_OX08B_CONFIG_1 3840
#define ALG_SDK_IMAGE_HEIGHT_OX08B_CONFIG_1 2160

#define ALG_SDK_IMAGE_SENSOR_TYPE_ISX021 3
#define ALG_SDK_IMAGE_SNESOR_CONFIG_TYPE_ISX021_CONFIG_1 1

#define ALG_SDK_IMAGE_DATA_TYPE_YUV422 1

#define ALG_SDK_HEAD_COMMON_TOPIC_NAME_LEN 128

#define ALG_SDK_ITEM_MANAGER_HEAD 66

#define ALG_SDK_TOPIC_NAME_IMAGE_DATA "/image_data/stream"
#define ALG_SDK_TOPIC_NAME_POC_INFO "/dev_info/poc_info"

#define ALG_SDK_CLIENT_THREAD_NUM_MAX 32

#define ALG_SDK_PAYLOAD_LEN_MAX 2*3840*2166

#define ALG_SDK_AMP_PROTOCAL_HEAD 0x5A
#define ALG_SDK_SERVICE_SENSOR_CONFIG_MAX_LINE 8192
#define ALG_SDK_AMP_PROTOCAL_PAYLOAD_SIZE 65519
#define ALG_SDK_SERVICE_SEND_BUFF_MAX_SIZE 65536
#define ALG_SDK_MAX_CHANNEL 16
#define ALG_SDK_MAX_DESERDES 8

#ifdef __cplusplus
extern "C" {
#endif

enum
{
    ALG_SDK_ITEM_MANAGER_DATA_TYPE_IMAGE = 1,
    ALG_SDK_ITEM_MANAGER_DATA_TYPE_POC_INFO,
    ALG_SDK_ITEM_MANAGER_DATA_TYPE_V1_MSGS,
};

enum
{
    ALG_SDK_SERVICE_SEND_TYPE_PC_WIN = 0,
    ALG_SDK_SERVICE_SEND_TYPE_PC_LINUX = 1,
    ALG_SDK_SERVICE_SEND_TYPE_PC_MAC = 2,
    ALG_SDK_SERVICE_SEND_TYPE_CAM_BOARD_1CH = 3,
    ALG_SDK_SERVICE_SEND_TYPE_CAM_BOARD_2CH = 4,
    ALG_SDK_SERVICE_SEND_TYPE_CAM_BOARD_4CH = 5,
    ALG_SDK_SERVICE_SEND_TYPE_CAM_BOARD_6CH = 6,
    ALG_SDK_SERVICE_SEND_TYPE_CAM_BOARD_8CH = 7,
    ALG_SDK_SERVICE_SEND_TYPE_CAM_BOARD_16CH = 8,
};

enum
{
    ALG_SDK_SERVICE_CMD_TYPE_COMMON = 0,
    ALG_SDK_SERVICE_CMD_TYPE_BOARD = 1,
    ALG_SDK_SERVICE_CMD_TYPE_SPECIAL = 0xFF,
};

enum
{
    ALG_SDK_SERVICE_PACKET_TYPE_RPL = 0,
    ALG_SDK_SERVICE_PACKET_TYPE_REQ = 1,
};

enum
{
    ALG_SDK_SERVICE_ACK_MODE_NO_REPLY = 0,
    ALG_SDK_SERVICE_ACK_MODE_NEED_REPLY = 1,
};

enum
{
    ALG_SDK_SERVICE_COMM_CMD_GET_ID = 0x0,
    ALG_SDK_SERVICE_COMM_CMD_SET_SEQ = 0x01,
    ALG_SDK_SERVICE_COMM_CMD_GET_HW_VERSION = 0x02,
    ALG_SDK_SERVICE_COMM_CMD_GET_SW_VERSION = 0x03,
    ALG_SDK_SERVICE_COMM_CMD_UPDATE_FIRMWARE = 0x04,
    ALG_SDK_SERVICE_COMM_CMD_READ_FILE = 0x05,
    ALG_SDK_SERVICE_COMM_CMD_GET_STATUS = 0x06,
    ALG_SDK_SERVICE_COMM_CMD_SET_TIME = 0x07,
    ALG_SDK_SERVICE_COMM_CMD_GET_TIME = 0x08,
};

enum
{
    ALG_SDK_SERVICE_PCIE_CMD_WRITE_REG = 0x02,
    ALG_SDK_SERVICE_PCIE_CMD_REAG_REG = 0x03,    
    ALG_SDK_SERVICE_PCIE_CMD_STREAM_CTL = 0x0e,
    ALG_SDK_SERVICE_PCIE_CMD_SENSOR_CONFIG = 0x10,
};

enum
{
    ALG_SDK_CMD_REPLY_ACK_CODE_SUCCESS = 0x00,
    ALG_SDK_CMD_REPLY_ACK_CODE_PARAM_ERR = 0xE0,
    ALG_SDK_CMD_REPLY_ACK_CODE_DEFAULT = 0xFF,
};

enum 
{
	ALG_SDK_SENSOR_TYPE_SONY_ISX021,
	ALG_SDK_SENSOR_TYPE_SONY_ISX019 = 1,
	ALG_SDK_SENSOR_TYPE_SONY_ISX031,
	ALG_SDK_SENSOR_TYPE_SONY_IMX390,
	ALG_SDK_SENSOR_TYPE_SONY_IMX424,
	ALG_SDK_SENSOR_TYPE_SONY_IMX490,
	ALG_SDK_SENSOR_TYPE_SONY_IMX728,

	ALG_SDK_SENSOR_TYPE_OV_OX1F10 = 10,
	ALG_SDK_SENSOR_TYPE_OV_OX3C,
	ALG_SDK_SENSOR_TYPE_OV_OX8B,
	ALG_SDK_SENSOR_TYPE_OV_OX5B1S,

	ALG_SDK_MAX_SENSOR_NUM,
};

enum
{
    ALG_SDK_MAX_GMSL_3G_MODE = 0x00,
    ALG_SDK_MAX_GMSL_6G_MODE = 0x01,
};

enum
{
    ALG_SDK_VIDEO_FORMAT_UNKOWN = 0,
    ALG_SDK_VIDEO_FORMAT_I420 = 2,          /* Planar YUY-420 format (YYYYYYYY-UU-VV) */
    ALG_SDK_VIDEO_FORMAT_YV12,              /* Planar YUY-420 format (like I420 but u-v swapped) */
    ALG_SDK_VIDEO_FORMAT_YUY2 = 4,          /* Packed YUY-422 format (Y-U-Y-V-Y-U-Y-V) */
    ALG_SDK_VIDEO_FORMAT_YUYV = ALG_SDK_VIDEO_FORMAT_YUY2,
    ALG_SDK_VIDEO_FORMAT_UYVY = 5,          /* Packed YUY-422 format (U-Y-V-Y-U-Y-V-Y) */
    ALG_SDK_VIDEO_FORMAT_VYUY = 64,         /* Packed YUY-422 format (V-Y-U-Y-V-Y-U-Y) */
    ALG_SDK_VIDEO_FORMAT_YVYU = 19,         /* Packed YUY-422 format (Y-V-Y-U-Y-V-Y-U) */
    ALG_SDK_VIDEO_FORMAT_RGB = 15,          /* RGB packed into 24 bits without padding (R-G-B-R-G-B) */
    ALG_SDK_VIDEO_FORMAT_RAW10 = 90,        /* RAW 10-bit */
};

enum
{
    ALG_SDK_MIPI_DATA_TYPE_DEFAULT = 0x00,  /* Default data type (2-bytes) */
    ALG_SDK_MIPI_DATA_TYPE_UYVY = 0x1C,     /* Type UYVY (2-bytes) */
    ALG_SDK_MIPI_DATA_TYPE_VYUY = 0x1D,     /* Type UYVY (2-bytes) */
    ALG_SDK_MIPI_DATA_TYPE_YUYV = 0x1E,     /* Type YUYV (2-bytes) */
    ALG_SDK_MIPI_DATA_TYPE_YVYU = 0x1F,     /* Type YVYU (2-bytes) */
    ALG_SDK_MIPI_DATA_TYPE_RAW10 = 0x2B,    /* Type RAW10 (1.25-bytes) */
    ALG_SDK_MIPI_DATA_TYPE_RAW12 = 0x2C,    /* Type RAW12 (1.5-bytes) */
};

enum
{
    ALG_SDK_TIME_MODE_UTC = 1 << 0,            /* use UTC Time */
    ALG_SDK_TIME_MODE_UNIX = 1 << 1,           /* use UNIX Time */
    ALG_SDK_TIME_MODE_RELATIVE_TIME = 1 << 2,  /* use Relative Time */
    ALG_SDK_TIME_MODE_ALL = 0x07,
};

typedef struct alg_sdk_pcie_common_head
{
    uint8_t     head;
    uint8_t     version;
    char        topic_name[128];
    uint8_t     crc8;
    uint8_t     resv[125];
}pcie_common_head_t;

typedef struct alg_sdk_pcie_image_info_meta
{
    uint32_t     frame_index;
    uint16_t     width;
    uint16_t     height;
    uint16_t     data_type;
    float        exposure;
    float        again;
    float        dgain;
    float        temp;
    size_t       img_size;
    uint64_t     timestamp;
}pcie_image_info_meta_t;

typedef struct alg_sdk_pcie_image_data
{
    pcie_common_head_t       common_head;
    pcie_image_info_meta_t   image_info_meta;
    void                     *payload;
}pcie_image_data_t;

typedef struct alg_sdk_pcie_poc_info_meta
{
    float        vol; 
    float        cur;
    uint64_t     timestamp;
}pcie_poc_info_meta_t;

typedef struct alg_sdk_pcie_poc_info
{
    pcie_common_head_t       common_head;
    pcie_poc_info_meta_t     poc_info_meta;
    // void                     *payload;
}pcie_poc_info_t;

/*  V1 Protocal */
typedef struct alg_sdk_pcie_msgs_meta
{
    uint8_t                 head;
    uint8_t                 version;
    uint8_t                 height;
    uint8_t                 data_type;
    size_t                  data_size;
    uint64_t                timestamp;
}pcie_msgs_meta_t;

typedef struct alg_sdk_service_get_time {
    uint8_t  ack_mode;

    uint64_t timestamp;
}service_get_time_t;

typedef struct alg_sdk_service_camera_config {
     /* Request Field */
    uint8_t  ack_mode;
    uint8_t  ch_id;
    uint16_t module_type;
    uint16_t width;
    uint16_t height;
    uint8_t  deser_mode;
    uint8_t  camera_num;
    uint8_t  data_type;
    uint16_t line_len;
    uint8_t  payload[7*ALG_SDK_SERVICE_SENSOR_CONFIG_MAX_LINE];

    /* Reply Field */
    uint8_t  ack_code;
    uint8_t  channel; 
}service_camera_config_t;

typedef struct alg_sdk_service_camera_read_reg {
    /* Request Field */
    uint8_t  ack_mode;
    uint8_t  ch_id;
    uint16_t msg_type; //0x1608 or 0x0808
    uint16_t device_addr;
    uint16_t line_len;
    uint16_t  payload[ALG_SDK_SERVICE_SENSOR_CONFIG_MAX_LINE];
 
    /* Reply Field */
    uint8_t  ack_code;
    uint8_t  channel;
    uint16_t length_r;
    uint16_t data[ALG_SDK_SERVICE_SENSOR_CONFIG_MAX_LINE];
}service_camera_read_reg_t;

typedef struct alg_sdk_service_camera_write_reg {
    /* Request Field */
    uint8_t  ack_mode;
    uint8_t  ch_id;
    uint16_t msg_type; //0x1608 or 0x0808
    uint16_t device_addr;
    uint16_t line_len;
    uint16_t payload[2*ALG_SDK_SERVICE_SENSOR_CONFIG_MAX_LINE];
 
    /* Reply Field */
    uint8_t  ack_code;
    uint8_t  channel;
}service_camera_write_reg_t;

typedef struct alg_sdk_service_stream_control{
    /* Request Field */
    uint8_t  ack_mode;
    uint8_t  select[ALG_SDK_MAX_CHANNEL];
    uint8_t  control[ALG_SDK_MAX_CHANNEL];

    /* Reply Field */
    uint8_t  ack_code;
    uint8_t  ch_sel[ALG_SDK_MAX_CHANNEL];
}service_stream_control_t;

typedef struct alg_sdk_amp_msg{
    uint8_t  head;
    uint8_t  version;
    uint8_t  send_type;
    uint8_t  send_id;
    uint8_t  crc8;
    uint8_t  receive_type;
    uint8_t  receive_id;
    uint16_t seq;
    uint8_t  ack_mode;
    uint8_t  packet_mode;
    uint8_t  cmd_type;
    uint8_t  cmd_id;
    uint16_t length;
    uint16_t crc16;
    uint8_t  payload[ALG_SDK_AMP_PROTOCAL_PAYLOAD_SIZE];
}alg_sdk_amp_msg_t;

typedef struct alg_sdk_task {
    alg_sdk_amp_msg_t amp_msg;
    struct alg_sdk_task* next;
    void *control;
}alg_sdk_task_t;

typedef struct alg_sdk_utc_time {
    uint16_t year : 12,      // max 4095
             month : 4;      // max 15
    uint8_t  day : 5,        // max 31
             weekday : 3;    // max 7
    uint8_t  hour;
    uint8_t  minute;
    uint8_t  second;
    uint32_t us;
}aili_utc_time_t;

typedef struct alg_sdk_service_set_time{
    /* Request Field */
    uint8_t  ack_mode;
    uint8_t  dev_index;
    uint8_t  time_mode;
    uint64_t unix_time;
    uint64_t relative_time;
    aili_utc_time_t utc_time;

    /* Reply Field */
    uint8_t  ack_code;
}service_set_time_t;

typedef struct
{
    uint8_t using_time_mode;
    aili_utc_time_t utc_time;
    uint64_t unix_time;
    uint64_t relative_time;
}aili_timestamp_ctl_t __attribute__ ((aligned(1)));

#ifdef __cplusplus
}
#endif
#endif
