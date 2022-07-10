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

void array_2_mat(uchar* data, int w, int h, int type, int ch_id, uint32_t frame_index, const char* image_name)
{
    if(type == ALG_IMG_TYPE_YUV)
    {
        /* Image Display */
        cv::Mat img = cv::Mat(h, w, CV_8UC2, data);
        cv::Mat dst = cv::Mat(h,w, CV_8UC3);
        cv::Mat rsz;
//        printf("H=%d W=%d CH=%d\n", img.size().height, img.size().width, img.channels());
        cv::cvtColor(img, dst, cv::COLOR_YUV2BGR_YUYV);
        cv::resize(dst, rsz, cv::Size(640,360));
        cv::imshow(image_name, rsz);

        /* Image Write */
        char c = cv::waitKey(1);
        if(c == 32)
        {
            /* save image */
            char filename[128] = {};
            sprintf(filename, "data/image_%02d_%08d.bmp", ch_id, frame_index);
//            printf("filename %s\n", filename);
            cv::imwrite(filename, dst);

            /* save raw data */
            char filename_2[128] = {};
            sprintf(filename_2, "data/image_%02d_%08d.raw", ch_id, frame_index);
            ofstream storage_file(filename_2,ios::out | ios::binary);
            storage_file.write((char *)data, h*w*2);
            storage_file.close();
        }
    }
    else if(type == ALG_IMG_TYPE_RAW10)
    {
        cv::Mat rsz;
        uint32_t data_size = w * h;
//        printf("W=%d H=%d size=%d,\n", w, h, data_size);

        ushort* pdata = (ushort*)malloc(sizeof(ushort) * data_size);
        for(int i = 0; i < int(data_size / 4); i++)
        {
            pdata[4*i] = (((((ushort)data[5*i]) << 2) & 0x03FC) | (ushort)((data[5*i+4] >> 0) & 0x0003));
            pdata[4*i+1] = (((((ushort)data[5*i+1]) << 2) & 0x03FC) | (ushort)((data[5*i+4] >> 2) & 0x0003));
            pdata[4*i+2] = (((((ushort)data[5*i+2]) << 2) & 0x03FC) | (ushort)((data[5*i+4] >> 4) & 0x0003));
            pdata[4*i+3] = (((((ushort)data[5*i+3]) << 2) & 0x03FC) | (ushort)((data[5*i+4] >> 6) & 0x0003));
        }

        /* Image Display */
        cv::Mat img_byer = cv::Mat(h, w, CV_16UC1, pdata);
        cv::Mat img_rgb8, img_rsz;
//        cv::cvtColor(img_byer, img_byer, cv::COLOR_BayerRG2BGR);
        cv::convertScaleAbs(img_byer, img_rgb8, 0.25, 0);
        cv::resize(img_rgb8, img_rsz, cv::Size(640,360));
        cv::imshow("Bayer", img_rsz);

        /* Image Write */
//        if(is_save_raw)
//        {
//            /* save raw data */
//            char filename[128] = {};
//            sprintf(filename, "data/image_%02d_%08d.raw", ch_id, frame_index);
//            ofstream storage_file(filename,ios::out | ios::binary);
//            storage_file.write((char *)data, data_size*2);
//            storage_file.close();
//        }

        char c = cv::waitKey(1);
        if(c == 32)
        {
            /* save image */
            char filename[128] = {};
            sprintf(filename, "image_%02d_%08d.bmp", ch_id, frame_index);
//            printf("filename %s\n", filename);
            cv::imwrite(filename, img_rgb8);
        }
        free(pdata);
    }
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
    strcpy(c_ch_id, ptr_topic);
    int ch_id = atoi(c_ch_id);

    return ch_id;
}

void callback_image_data(void *p)
{
    pcie_image_data_t* msg = (pcie_image_data_t*)p;
    /* Debug message */
//     printf("[frame = %d], [len %ld], [byte_0 = %d], [byte_end = %d]\n",
//            msg->image_info_meta.frame_index,  msg->image_info_meta.img_size, ((uint8_t*)msg->payload)[0], ((uint8_t*)msg->payload)[msg->image_info_meta.img_size - 1]);

    /* check frame rate (every 1 second) */
    frame_rate_monitor(get_channel_id(msg), msg->image_info_meta.frame_index);

    /* for Image Display (by OpenCV)
        This may cause frame rate drop when CPU has run out of resources. 
    */
    array_2_mat((uchar*)msg->payload, msg->image_info_meta.width, msg->image_info_meta.height, ALG_IMG_TYPE_YUV, get_channel_id(msg), msg->image_info_meta.frame_index, msg->common_head.topic_name);  // YUV422 type is CV_8U2
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
        const char* topic_name = "/image_data/stream";

        rc = alg_sdk_subscribe(topic_name, callback_image_data);
        if (rc < 0)
        {
            printf("Subscribe to topic %s Error!\n", topic_name);
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
