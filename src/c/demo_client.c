#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <sys/time.h>
#include "alg_sdk/client.h"
#include "alg_common/basic_types.h"

#ifdef __BUILD_IMAGE_DISP__
    #include "opencv/cv.h"
    #include "opencv2/core.hpp"
    #include "opencv2/imgproc.hpp"
    #include "opencv2/imgcodecs.hpp"
    #include "opencv2/highgui.hpp"
#endif
const char topic_image_head_d[ALG_SDK_HEAD_COMMON_TOPIC_NAME_LEN] = {ALG_SDK_TOPIC_NAME_IMAGE_DATA};
const char topic_dev_poc_head_d[ALG_SDK_HEAD_COMMON_TOPIC_NAME_LEN] = {ALG_SDK_TOPIC_NAME_POC_INFO};

static uint64_t g_t_last[ALG_SDK_MAX_CHANNEL] = {0};
static int g_f_count[ALG_SDK_MAX_CHANNEL] = {0};
static uint32_t g_f_last[ALG_SDK_MAX_CHANNEL] = {0};

#ifdef __BUILD_IMAGE_DISP__
void array_2_mat(uchar* data, int w, int h, int type, const char* view_name)
{
    cv::Mat img = cv::Mat(h, w, type, data);
    cv::Mat dst = cv::Mat(h,w, CV_8UC3);
    cv::Mat rsz;

    if(type == CV_8UC2)
    {
        cv::cvtColor(img, dst, cv::COLOR_YUV2BGR_YUYV);
        cv::resize(dst, rsz, cv::Size(640,360));
        // printf("H=%d W=%d CH=%d\n", dst.size().height, dst.size().width, dst.channels());
        cv::imshow(view_name, rsz);
        cv::waitKey(1);
    }
}
#endif

/*  Return the UNIX time in milliseconds.  You'll need a working
    gettimeofday(), so this won't work on Windows.  */
uint64_t milliseconds (void)
{
    struct timeval tv;
    gettimeofday (&tv, NULL);
    return (((uint64_t)tv.tv_sec * 1000) + ((uint64_t)tv.tv_usec / 1000));
}

void frame_rate_monitor(const int ch_id, const int frame_index)
{
    /* Current frame should be last frame +1, 
        otherwise some frames may be lost.
    */
    if(( (frame_index - g_f_last[ch_id]) <= 1) || (g_f_last[ch_id] == 0) )
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
    int delta_t = t_now - g_t_last[ch_id];
    if (delta_t > 1000)  // for 1000 milliseconds
    {
        g_t_last[ch_id] = t_now;
        printf("Frame Monitor : [Channel %d] [Index %d] [Frame Rate = %f]\n", ch_id, frame_index, (float)g_f_count[ch_id] / delta_t * 1000.0f);
        g_f_count[ch_id] = 0;
    }
}

int get_channel_id(const pcie_image_data_t* msg)
{
    const char* ptr_topic =  &msg->common_head.topic_name[19];
    char c_ch_id[2];
    strcpy(c_ch_id, ptr_topic);
    int ch_id = atoi(c_ch_id);

    return ch_id;
}

void callback_image_data(void *p)
{
    pcie_image_data_t* msg = (pcie_image_data_t*)p;
    printf("[frame = %d], [time %ld], [byte_0 = %d], [byte_end = %d]\n",
           msg->image_info_meta.frame_index,  msg->image_info_meta.timestamp, ((uint8_t*)msg->payload)[0], ((uint8_t*)msg->payload)[msg->image_info_meta.img_size - 1]);
}

void callback_poc_info(void *p)
{
    pcie_poc_info_t* msg = (pcie_poc_info_t*)p;

    printf("[time : %ld], [cur = %f], [vol = %f]\n",
           msg->poc_info_meta.timestamp, msg->poc_info_meta.cur, msg->poc_info_meta.vol);
}


void int_handler(int sig)
{
    printf("Caught signal : %d\n", sig);
    alg_sdk_stop_client();

    /* terminate program */
    exit(sig);
}

int main (int argc, char **argv)
{
    signal(SIGINT, int_handler);

    if ((argc == 2) && (strcmp(argv[1], "-all") == 0))
    {
        int rc;

        printf("Client subscribe to topic [%s] and [%s]\n", topic_image_head_d, topic_dev_poc_head_d);
        rc = alg_sdk_subscribe(topic_image_head_d, callback_image_data);
        if (rc < 0)
        {
            printf("Subscribe to topic %s Error!\n", topic_image_head_d);
            exit(0);
        }
        rc = alg_sdk_subscribe(topic_dev_poc_head_d, callback_poc_info);
        if (rc < 0)
        {
            printf("Subscribe to topic %s Error!\n", topic_dev_poc_head_d);
            exit(0);
        }

        if (alg_sdk_init_client())
        {
            printf("Init Client Error!\n");
            exit(0);
        }

        alg_sdk_client_spin_on();
    }
    else if ((argc == 3) && (strcmp (argv[1], "-image") == 0))
    {
        int rc;
        const char* client_name = "client0";
        const char* topic_name = argv[2];
        printf("Client [%s] subscribe to topic [%s]\n", client_name, topic_name);

        if(strncmp(topic_name, topic_image_head_d, strlen(topic_image_head_d)) == 0)
        {
            rc = alg_sdk_subscribe(topic_name, callback_image_data);
            if (rc < 0)
            {
                printf("Subscribe to topic %s Error!\n", topic_name);
                exit(0);
            }
        }

        if(alg_sdk_init_client())
        {
            printf("Init Client Error!\n");
            exit(0);
        }

        alg_sdk_client_spin_on();
    }
    else if ((argc == 2) && (strcmp (argv[1], "-all_images") == 0))
    {
        int rc;
        const char* topic_name_1 = "/image_data/stream/00";
        const char* topic_name_2 = "/image_data/stream/02";
        const char* topic_name_3 = "/image_data/stream/04";
        const char* topic_name_4 = "/image_data/stream/06";

        rc = alg_sdk_subscribe(topic_name_1, callback_image_data);
        if (rc < 0)
        {
            printf("Subscribe to topic %s Error!\n", topic_name_1);
            exit(0);
        }
        rc = alg_sdk_subscribe(topic_name_2, callback_image_data);
        if (rc < 0)
        {
            printf("Subscribe to topic %s Error!\n", topic_name_2);
            exit(0);
        }
        rc = alg_sdk_subscribe(topic_name_3, callback_image_data);
        if (rc < 0)
        {
            printf("Subscribe to topic %s Error!\n", topic_name_3);
            exit(0);
        }
        rc = alg_sdk_subscribe(topic_name_4, callback_image_data);
        if (rc < 0)
        {
            printf("Subscribe to topic %s Error!\n", topic_name_4);
            exit(0);
        }
        if(alg_sdk_init_client())
        {
            printf("Init Client Error!\n");
            exit(0);
        }

        alg_sdk_client_spin_on();
    }
    return 0;
}
