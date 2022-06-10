#include <stdio.h>
#include <stdlib.h>

#include "alg_sdk/client.h"

#include "opencv/cv.h"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"

const char topic_image_head_d[ALG_SDK_HEAD_COMMON_TOPIC_NAME_LEN] = {ALG_SDK_TOPIC_NAME_IMAGE_DATA};

void array_2_mat(uchar* data, int w, int h, int type)
{
    cv::Mat img = cv::Mat(h, w, type, data);
    cv::Mat dst = cv::Mat(h,w, CV_8UC3);
    cv::Mat rsz;

    if(type == CV_8UC2)
    {
        cv::cvtColor(img, dst, cv::COLOR_YUV2BGR_YUYV);
        cv::resize(dst, rsz, cv::Size(640,360));
        printf("H=%d W=%d CH=%d\n", dst.size().height, dst.size().width, dst.channels());
        cv::imshow("view", rsz);
        cv::waitKey(10);
    }
}

void callback_image_data(void *p)
{
    pcie_image_data_t* msg = (pcie_image_data_t*)p;
    printf("[frame = %d], [time %ld], [byte_0 = %d], [byte_end = %d]\n",
           msg->image_info_meta.frame_index,  msg->image_info_meta.timestamp, ((uint8_t*)msg->payload)[0], ((uint8_t*)msg->payload)[msg->image_info_meta.img_size - 1]);

    array_2_mat((uchar*)msg->payload, msg->image_info_meta.width, msg->image_info_meta.height, CV_8UC2);  // YUV422 type is CV_8U2
}

int main (int argc, char **argv)
{
    if ((argc == 3) && (strcmp (argv[1], "-c") == 0))
    {
        int rc;
        const char* topic_name = argv[2];
        alg_sdk_log(LOG_VERBOSE, "subscribe to topic [%s]\n", topic_name);

        if(strncmp(topic_name, topic_image_head_d, strlen(topic_image_head_d)) == 0)
        {
            rc = alg_sdk_subscribe(topic_name, callback_image_data);
            if (rc < 0)
            {
                alg_sdk_log(LOG_ERROR, "Subscribe to topic %s Error!\n", topic_name);
                // return 0;
            }
        }
        else
        {
            alg_sdk_log(LOG_ERROR, "Subscribe to topic %s Error!\n", topic_name);
            exit(0);
        }

        if(alg_sdk_init_client())
        {
            alg_sdk_log(LOG_ERROR, "Init Client Error!\n");
            exit(0);
        }
        /* Do something here. */
        alg_sdk_stop_client();
        alg_sdk_log(LOG_DEBUG, "Stop Client.");
    }

    return 0;
}
