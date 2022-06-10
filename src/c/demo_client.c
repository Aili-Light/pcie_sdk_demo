#include <stdio.h>
#include <stdlib.h>

#include "alg_sdk/client.h"

const char topic_image_head_d[ALG_SDK_HEAD_COMMON_TOPIC_NAME_LEN] = {ALG_SDK_TOPIC_NAME_IMAGE_DATA};
const char topic_dev_poc_head_d[ALG_SDK_HEAD_COMMON_TOPIC_NAME_LEN] = {ALG_SDK_TOPIC_NAME_POC_INFO};

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

int main (int argc, char **argv)
{
    if ((argc == 2) && (strcmp(argv[1], "-c") == 0))
    {
        int rc;
        // alg_sdk_init_client();

        alg_sdk_log(LOG_VERBOSE, "Client [%s] subscribe to topic [%s] and [%s]\n", topic_image_head_d, topic_dev_poc_head_d);
        rc = alg_sdk_subscribe(topic_image_head_d, callback_image_data);
        if (rc < 0)
        {
            alg_sdk_log(LOG_ERROR, "Subscribe to topic %s Error!\n", topic_image_head_d);
            // return 0;
        }
        rc = alg_sdk_subscribe(topic_dev_poc_head_d, callback_poc_info);
        if (rc < 0)
        {
            alg_sdk_log(LOG_ERROR, "Subscribe to topic %s Error!\n", topic_dev_poc_head_d);
            // return 0;
        }

        if (alg_sdk_init_client())
        {
            alg_sdk_log(LOG_ERROR, "Init Client Error!\n");
            exit(0);
        }
        /* Do something here. */

        alg_sdk_stop_client();
        alg_sdk_log(LOG_DEBUG, "Stop Client.");
    }
    else if ((argc == 4) && (strcmp (argv[1], "-c") == 0))
    {
        int rc;
        const char* client_name = argv[2];
        const char* topic_name = argv[3];
        alg_sdk_log(LOG_VERBOSE, "Client [%s] subscribe to topic [%s]\n", client_name, topic_name);

        if(strncmp(topic_name, topic_image_head_d, strlen(topic_image_head_d)) == 0)
        {
            rc = alg_sdk_subscribe(topic_name, callback_image_data);
            if (rc < 0)
            {
                alg_sdk_log(LOG_ERROR, "Subscribe to topic %s Error!\n", topic_name);
                // return 0;
            }
        }
        else if(strncmp(topic_name, topic_dev_poc_head_d, strlen(topic_dev_poc_head_d)) == 0)
        {
            rc = alg_sdk_subscribe(topic_name, callback_poc_info);
            if (rc < 0)
            {
                alg_sdk_log(LOG_ERROR, "Subscribe to topic %s Error!\n", topic_name);
                // return 0;
            }
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
