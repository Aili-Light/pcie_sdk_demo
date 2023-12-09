#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdint.h>
#include <sys/time.h>

#include "alg_sdk/client.h"
#include "alg_common/basic_types.h"
#include <fstream>
#include "alg_cvt/alg_cvtColor.h"
#include "alg_isp/alg_isp_pipeline.h"

#ifdef WITH_ROS
#include "alg_rosbridge/alg_rospub.h"
#elif WITH_ROS2
#include "alg_ros2bridge/alg_ros2pub.h"
#endif

#define ALG_SDK_PAYLOAD_RGB_MAX 3 * 4096 * 4096
using namespace std;
const char topic_image_head_d[ALG_SDK_HEAD_COMMON_TOPIC_NAME_LEN] = {ALG_SDK_TOPIC_NAME_IMAGE_DATA};

static uint64_t g_t_last[ALG_SDK_MAX_CHANNEL] = {0};
static int g_f_count[ALG_SDK_MAX_CHANNEL] = {0};
static uint32_t g_f_last[ALG_SDK_MAX_CHANNEL] = {0};
static uint8_t g_buffer_rgb[ALG_SDK_MAX_CHANNEL][ALG_SDK_PAYLOAD_RGB_MAX];
static uint8_t g_buffer_rgb_awb[ALG_SDK_MAX_CHANNEL][ALG_SDK_PAYLOAD_RGB_MAX];
#ifdef WITH_ROS
static AlgRosPubNode g_rospub[ALG_SDK_MAX_CHANNEL];
#elif WITH_ROS2
static AlgRos2PubNode g_rospub[ALG_SDK_MAX_CHANNEL];
#endif

#define ALG_IMG_TYPE_YUV 0x01
#define ALG_IMG_TYPE_RAW10 0x2B

/*  Return the UNIX time in milliseconds.  You'll need a working
    gettimeofday(), so this won't work on Windows.  */
uint64_t milliseconds(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (((uint64_t)tv.tv_sec * 1000) + ((uint64_t)tv.tv_usec / 1000));
}

void int_handler(int sig)
{
    printf("Caught signal : %d\n", sig);

#ifdef WITH_ROS
    ros::shutdown();
#elif WITH_ROS2
    rclcpp::shutdown();
#endif

    alg_sdk_stop_client();
    /* terminate program */
    exit(sig);
}

void array_2_mat(uint8_t *data, int w, int h, int data_type, int ch_id, uint32_t frame_index, uint64_t timestamp, const char *image_name)
{
    const uint32_t data_size = w * h;
    const uint32_t data_size_rgb = data_size * 3;
    uint8_t* buf_rgb = (uint8_t*)&g_buffer_rgb[ch_id];
    uint8_t* buf_rgb_awb= (uint8_t*)&g_buffer_rgb_awb[ch_id];

#ifdef WITH_ROS
    AlgRosPubNode* ros_pub = &g_rospub[ch_id];
#elif WITH_ROS2
    AlgRos2PubNode* ros_pub = &g_rospub[ch_id];
#endif

    if (data_type <= ALG_SDK_MIPI_DATA_TYPE_YVYU)
    {
        /* Image Display */
        /* yuv to rgb conversion */
        alg_cv::alg_sdk_cvtColor((uint8_t *)data, (uint8_t *)buf_rgb, w, h, alg_cv::ALG_CV_YUV2RGBCVT_YUYV);
        ros_pub->PublishImage(frame_index, image_name, h, w, ALG_SDK_VIDEO_FORMAT_RGB, data_size_rgb, timestamp, buf_rgb);
    }
    else if (data_type == ALG_SDK_MIPI_DATA_TYPE_RAW12)
    {
        /* raw to rgb conversion */
        /* PCIE RAW Data Conversion */
        uint8_t *pdata = (uint8_t *)malloc(sizeof(uint8_t) * data_size);
        for (int i = 0; i < int(data_size / 2); i++)
        {
            pdata[2 * i] = ((((data[3 * i]) << 4) & 0x0FF0) | ((data[3 * i + 2] >> 0) & 0x000F)) >> 4;
            pdata[2 * i + 1] = ((((data[3 * i + 1]) << 4) & 0x0FF0) | ((data[3 * i + 2] >> 4) & 0x000F)) >> 4;
        }
        /* End - PCIE RAW Data Conversion */
        /* Demosaic */
        alg_cv::alg_sdk_cvtColor(pdata, buf_rgb, w, h, alg_cv::ALG_CV_BayerGB2RGB);
        alg_cv::alg_sdk_awb(buf_rgb, buf_rgb_awb, w, h, alg_cv::ALG_CV_AWB_DYM_THRESHOLD);
        ros_pub->PublishImage(frame_index, image_name, h, w, ALG_SDK_VIDEO_FORMAT_RGB, data_size_rgb, timestamp, buf_rgb_awb);
        free(pdata);
    }
}

void frame_rate_monitor(const int ch_id, const int frame_index)
{
    /* Current frame should be last frame +1,
        otherwise some frames may be lost.
    */
    if (((frame_index - g_f_last[ch_id]) <= 1) || (g_f_last[ch_id] == 0))
    {
        g_f_last[ch_id] = frame_index;
    }
    else
    {
        // printf("Frame Monitor : Frame drop ! [Current Frame %d] [Last Frame %d]\n", frame_index, g_f_last[ch_id]);
        g_f_last[ch_id] = frame_index;
    }

    uint64_t t_now = milliseconds();
    g_f_count[ch_id]++;
    uint64_t delta_t = t_now - g_t_last[ch_id];
    if (delta_t > 1000) // for 1000 milliseconds
    {
        g_t_last[ch_id] = t_now;
        printf("Frame Monitor : [Channel %d] [Index %d] [Frame Rate = %f]\n", ch_id, frame_index, (float)g_f_count[ch_id] / delta_t * 1000.0f);
        g_f_count[ch_id] = 0;
    }
}

int get_channel_id(const char *topic_name)
{
    const char *ptr_topic = &topic_name[19];
    char c_ch_id[2];
    strncpy(c_ch_id, ptr_topic, 2);
    int ch_id = atoi(c_ch_id);

    return ch_id;
}

void callback_image_data(void *p)
{
    pcie_image_data_t *msg = (pcie_image_data_t *)p;
    /* Debug message */
    int ch_id = get_channel_id(msg->common_head.topic_name);
    // printf("[frame = %d], [len %d], [byte_0 = %d], [byte_end = %d]\n",
    //        msg->image_info_meta.frame_index, msg->image_info_meta.img_size, ((uint8_t *)msg->payload)[0], ((uint8_t *)msg->payload)[msg->image_info_meta.img_size - 1]);

    /* check frame rate (every 1 second) */
    frame_rate_monitor(ch_id, msg->image_info_meta.frame_index);

    /* for Image Display (by OpenCV)
        This may cause frame rate drop when CPU has run out of resources.
    */
    array_2_mat((uint8_t *)msg->payload, msg->image_info_meta.width, msg->image_info_meta.height, msg->image_info_meta.data_type, ch_id, msg->image_info_meta.frame_index, msg->image_info_meta.timestamp, msg->common_head.topic_name); // YUV422 type is CV_8U2
}

int main(int argc, char *argv[])
{
    signal(SIGINT, int_handler);
#ifdef WITH_ROS
    ros::init(argc, argv, "alg_sdk_ros_publisher", ros::init_options::NoSigintHandler);
    ros::Time::init();
#elif WITH_ROS2
    rclcpp::init(argc, argv);
#endif

    if ((argc == 3) && (strcmp(argv[1], "-c") == 0))
    {
        int rc;
        const char *topic_name = argv[2];
        printf("subscribe to topic [%s]\n", topic_name);

        /* initialize ros */ 
        int ch_id = get_channel_id(topic_name);
        printf("Init rospub on ch [%d]\n", ch_id);
#ifdef WITH_ROS
        AlgRosPubNode* ros_pub = &g_rospub[ch_id];
        ros_pub->Init(ch_id);
#elif WITH_ROS2
        AlgRos2PubNode* ros_pub = &g_rospub[ch_id];
        ros_pub->Init(ch_id);
#endif

        /* Check the head of topic name */
        if (strncmp(topic_name, topic_image_head_d, strlen(topic_image_head_d)) == 0)
        {
            rc = alg_sdk_subscribe(topic_name, callback_image_data);
            if (rc < 0)
            {
                printf("Subscribe to topic %s Error!\n", topic_name);
                // return 0;
            }
        }
        else
        {
            printf("Subscribe to topic %s Error!\n", topic_name);
            exit(0);
        }

        if (alg_sdk_init_client())
        {
            printf("Init Client Error!\n");
            exit(0);
        }
        // alg_sdk_stop_client();
        alg_sdk_client_spin_on();
    }
    else if ((argc == 2) && (strcmp(argv[1], "-all") == 0))
    {
        int rc;
        char image_topic_names[ALG_SDK_MAX_CHANNEL][256];
        for (int i = 0; i < ALG_SDK_MAX_CHANNEL; i++)
        {
            snprintf(image_topic_names[i], 256, "/image_data/stream/%02d", i);
            printf("Client [%02d] subscribe to topic [%s]\n", i, image_topic_names[i]);

            printf("Init rospub on ch [%d]\n", i);
#ifdef WITH_ROS
            AlgRosPubNode* ros_pub = &g_rospub[i];
            ros_pub->Init(i);
#elif WITH_ROS2
            AlgRos2PubNode* ros_pub = &g_rospub[i];
            ros_pub->Init(i);
#endif
            rc = alg_sdk_subscribe(image_topic_names[i], callback_image_data);
            if (rc < 0)
            {
                printf("Subscribe to topic %s Error!\n", image_topic_names[i]);
                exit(0);
            }
        }

        if (alg_sdk_init_client())
        {
            printf("Init Client Error!\n");
            exit(0);
        }

        alg_sdk_client_spin_on();
    }


    return 0;
}