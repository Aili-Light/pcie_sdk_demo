#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdint.h>
#include <sys/time.h>

#include "alg_sdk/client.h"
#include "alg_common/basic_types.h"

#include "opencv/cv.h"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <fstream>
#include "alg_cvt/alg_cvtColor.h"

using namespace std;
const char topic_image_head_d[ALG_SDK_HEAD_COMMON_TOPIC_NAME_LEN] = {ALG_SDK_TOPIC_NAME_IMAGE_DATA};

static uint64_t g_t_last[ALG_SDK_MAX_CHANNEL] = {0};
static int g_f_count[ALG_SDK_MAX_CHANNEL] = {0};
static uint32_t g_f_last[ALG_SDK_MAX_CHANNEL] = {0};

#define ALG_IMG_TYPE_YUV 0x01
#define ALG_IMG_TYPE_RAW10 0x2B

/*  Return the UNIX time in milliseconds.  You'll need a working
    gettimeofday(), so this won't work on Windows.  */
uint64_t milliseconds (void)
{
    struct timeval tv;
    gettimeofday (&tv, NULL);
    return (((uint64_t)tv.tv_sec * 1000) + ((uint64_t)tv.tv_usec / 1000));
}

void int_handler(int sig)
{
    // printf("Caught signal : %d\n", sig);
    alg_sdk_stop_client();

    /* terminate program */
    exit(sig);
}

void array_2_mat(uchar* data, int w, int h, int data_type, int ch_id, uint32_t frame_index, const char* image_name)
{
    const uint32_t data_size = w * h;
    cv::Mat img_rgb8 = cv::Mat(h, w, CV_8UC3);
    if(data_type <= ALG_SDK_MIPI_DATA_TYPE_YVYU)
    {
        /* Image Display */
        uchar* buf_rgb = img_rgb8.data;
        /* yuv to rgb conversion */
        alg_cv::alg_sdk_cvtColor((uchar*)data, (uchar*)buf_rgb, w, h, alg_cv::image_format(data_type));
        /* Image Display */
//        cv::resize(img_rgb8, img_rgb8, cv::Size(1280,720));
        cv::namedWindow(image_name, CV_WINDOW_NORMAL);
        cv::imshow(image_name, img_rgb8);
    }
    else if(data_type == ALG_SDK_MIPI_DATA_TYPE_RAW10)
    {
        /* raw to rgb conversion */
        /* PCIE RAW Data Conversion */
        ushort* pdata = (ushort*)malloc(sizeof(ushort) * data_size);
        for(int i = 0; i < int(data_size / 4); i++)
        {
            pdata[4*i] = (((((ushort)data[5*i]) << 2) & 0x03FC) | (ushort)((data[5*i+4] >> 0) & 0x0003));
            pdata[4*i+1] = (((((ushort)data[5*i+1]) << 2) & 0x03FC) | (ushort)((data[5*i+4] >> 2) & 0x0003));
            pdata[4*i+2] = (((((ushort)data[5*i+2]) << 2) & 0x03FC) | (ushort)((data[5*i+4] >> 4) & 0x0003));
            pdata[4*i+3] = (((((ushort)data[5*i+3]) << 2) & 0x03FC) | (ushort)((data[5*i+4] >> 6) & 0x0003));
        }
        /* End - PCIE RAW Data Conversion */
        /* Demosaic */
        cv::Mat img_byer = cv::Mat(h, w, CV_16UC1, pdata);
        cv::convertScaleAbs(img_byer, img_byer, 0.25, 0);
        cv::Mat img_rgb8;
        cv::cvtColor(img_byer, img_rgb8, cv::COLOR_BayerBG2RGB);
        /* Image Display */
//        cv::resize(img_rgb8, img_rgb8, cv::Size(1280,720));
        cv::namedWindow(image_name, CV_WINDOW_NORMAL);
        cv::imshow(image_name, img_rgb8);
        free(pdata);
    }
    else if(data_type == ALG_SDK_MIPI_DATA_TYPE_RAW12)
    {
        /* raw to rgb conversion */
        /* PCIE RAW Data Conversion */
        ushort* pdata = (ushort*)malloc(sizeof(ushort) * data_size);
        for(int i = 0; i < int(data_size / 2); i++)
        {
            pdata[2*i] = (((((ushort)data[3*i]) << 4) & 0x0FF0) | (ushort)((data[3*i+2] >> 0) & 0x000F));
            pdata[2*i+1] = (((((ushort)data[3*i+1]) << 4) & 0x0FF0) | (ushort)((data[3*i+2] >> 4) & 0x000F));
        }
        /* End - PCIE RAW Data Conversion */
        /* Demosaic */
        cv::Mat img_byer = cv::Mat(h, w, CV_16UC1, pdata);
        cv::convertScaleAbs(img_byer, img_byer, 0.0625, 0);
        cv::Mat img_rgb8;
        cv::cvtColor(img_byer, img_rgb8, cv::COLOR_BayerBG2RGB);
        /* Image Display */
//        cv::resize(img_rgb8, img_rgb8, cv::Size(1280,720));
        cv::namedWindow(image_name, CV_WINDOW_NORMAL);
        cv::imshow(image_name, img_rgb8);
        free(pdata);
    }
    /* Image Data Write */
    char c = cv::waitKey(1);
    // if(c == 32)
    // {
    //     /* save raw data */
    //     char filename_2[128] = {};
    //     sprintf(filename_2, "data/image_%02d_%08d.raw", ch_id, frame_index);
    //     ofstream storage_file(filename_2,ios::out | ios::binary);
    //     storage_file.write((char *)data, data_size*2);
    //     storage_file.close();

    //     /* save image */
    //     char filename[128] = {};
    //     sprintf(filename, "data/image_%02d_%08d.bmp", ch_id, frame_index);
    //     cv::imwrite(filename, img_rgb8);
    // }
    
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
    uint64_t delta_t = t_now - g_t_last[ch_id];
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
    strncpy(c_ch_id, ptr_topic, 2);
    int ch_id = atoi(c_ch_id);

    return ch_id;
}

void callback_image_data(void *p)
{
    pcie_image_data_t* msg = (pcie_image_data_t*)p;
    /* Debug message */
    // printf("[frame = %d], [len %ld], [byte_0 = %d], [byte_end = %d]\n",
    //        msg->image_info_meta.frame_index,  msg->image_info_meta.img_size, ((uint8_t*)msg->payload)[0], ((uint8_t*)msg->payload)[msg->image_info_meta.img_size - 1]);

    /* check frame rate (every 1 second) */
    frame_rate_monitor(get_channel_id(msg), msg->image_info_meta.frame_index);

    /* for Image Display (by OpenCV)
        This may cause frame rate drop when CPU has run out of resources. 
    */
   array_2_mat((uchar*)msg->payload, msg->image_info_meta.width, msg->image_info_meta.height, msg->image_info_meta.data_type, get_channel_id(msg), msg->image_info_meta.frame_index, msg->common_head.topic_name);  // YUV422 type is CV_8U2
}

int main (int argc, char **argv)
{
    signal(SIGINT, int_handler);

    if ((argc == 3) && (strcmp (argv[1], "-c") == 0))
    {
        int rc;
        const char* topic_name = argv[2];
        printf("subscribe to topic [%s]\n", topic_name);

        /* Check the head of topic name */
        if(strncmp(topic_name, topic_image_head_d, strlen(topic_image_head_d)) == 0)
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

        if(alg_sdk_init_client())
        {
            printf("Init Client Error!\n");
            exit(0);
        }
        // alg_sdk_stop_client();
        alg_sdk_client_spin_on();
    }
    else if ((argc == 2) && (strcmp (argv[1], "-all") == 0))
    {
        int rc;
        char image_topic_names[ALG_SDK_MAX_CHANNEL][256];
        for (int i = 0; i < ALG_SDK_MAX_CHANNEL; i++)
        {
            snprintf(image_topic_names[i], 256, "/image_data/stream/%02d", i);
            printf("Client [%02d] subscribe to topic [%s]\n", i, image_topic_names[i]);

            rc = alg_sdk_subscribe(image_topic_names[i], callback_image_data);
            if (rc < 0)
            {
                printf("Subscribe to topic %s Error!\n", image_topic_names[i]);
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
    return 0;
}
