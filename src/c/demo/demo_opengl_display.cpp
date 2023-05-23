#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdint.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include "alg_sdk/client.h"
#include "alg_common/basic_types.h"
#include "alg_camera/alg_camera.h"

#define ALG_SDK_IMAGE_DATA_BUFF_LEN 8

const char topic_image_head_d[ALG_SDK_HEAD_COMMON_TOPIC_NAME_LEN] = {ALG_SDK_TOPIC_NAME_IMAGE_DATA};

static pthread_t g_camera_disp_thred[ALG_SDK_MAX_CHANNEL];
static pthread_mutex_t mutex[ALG_SDK_MAX_CHANNEL];
static sem_t full[ALG_SDK_MAX_CHANNEL];
static uint64_t g_t_last[ALG_SDK_MAX_CHANNEL] = {0};
static uint32_t g_f_count[ALG_SDK_MAX_CHANNEL] = {0};
uint64_t g_tlast = 0;
bool g_signal_recieved = false;

typedef struct camera_disp_args
{
    void *arg;
    void *ch_id;
} camera_disp_args_t;

camera_disp_args_t g_camera_args[ALG_SDK_MAX_CHANNEL];
AlgCamera g_camera[ALG_SDK_MAX_CHANNEL];

void int_handler(int sig)
{
    g_signal_recieved = True;

    for (int i = 0; i < ALG_SDK_MAX_CHANNEL; i++)
    {
        g_camera[i].close_camera();
    }

    for (int i = 0; i < ALG_SDK_MAX_CHANNEL; i++)
    {
        pthread_mutex_destroy(&mutex[i]);
        sem_destroy(&full[i]);
    }
    /* terminate program */
    exit(sig);
}

int get_channel_id(const pcie_image_data_t *msg)
{
    const char *ptr_topic = &msg->common_head.topic_name[19];
    char c_ch_id[2];
    strcpy(c_ch_id, ptr_topic);
    int ch_id = atoi(c_ch_id);

    return ch_id;
}

void callback_image_data(void *p)
{
    pcie_image_data_t *msg = (pcie_image_data_t *)p;

    int ch_id = get_channel_id(msg);
    // printf("[channel = %d], [frame = %d], [time %ld], [byte_0 = %d], [byte_end = %d]\n", ch_id,
    // msg->image_info_meta.frame_index,  msg->image_info_meta.timestamp, ((uint8_t*)msg->payload)[0], ((uint8_t*)msg->payload)[msg->image_info_meta.img_size - 1]);
    // printf("[channel = %d], [frame = %d], [time %ld], [Exp = %f], [AGain = %f], [DGain = %f]\n", ch_id,
    // msg->image_info_meta.frame_index,  msg->image_info_meta.timestamp,  msg->image_info_meta.exposure, msg->image_info_meta.again, msg->image_info_meta.dgain);

    g_camera[ch_id].capture_image(msg);
    sem_post(&full[ch_id]);
}

void *camera_display_thread(void *arg)
{
    camera_disp_args_t *t = (camera_disp_args_t *)arg;
    AlgCamera *camera = (AlgCamera *)t->arg;
    int ch_id = (intptr_t)t->ch_id;

    while (!g_signal_recieved)
    {
        sem_wait(&full[ch_id]);
        if (!camera->is_init())
        {
            printf("Init camera [%d]\n", ch_id);
            camera->init_camera(ch_id, ALG_CAMERA_FLAG_SOURCE_PCIE);
        }
        camera->img_converter();
        camera->render_image();

        if (camera->is_closed())
            g_signal_recieved = true;
    }
}

int main(int argc, char **argv)
{
    /* Create thread */
    for (int i = 0; i < ALG_SDK_MAX_CHANNEL; i++)
    {
        camera_disp_args_t arg = {(void *)(AlgCamera *)&g_camera[i], (void *)(intptr_t)i};
        g_camera_args[i] = arg;
    }

    memset(g_camera_disp_thred, 0, sizeof(g_camera_disp_thred));

    for (int i = 0; i < ALG_SDK_MAX_CHANNEL; i++)
    {
        pthread_mutex_init(&mutex[i], NULL);
        sem_init(&full[i], 0, 0);

        int rc;
        rc = pthread_create(&g_camera_disp_thred[i], NULL, camera_display_thread, (void *)(&g_camera_args[i]));
        if (rc < 0)
        {
            printf("Server Thread Create Error: %s\n", strerror(rc));
            return 1;
        }
    }

    if ((argc == 2) && (strcmp(argv[1], "-all") == 0))
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
    }
    else if ((argc == 3) && (strcmp(argv[1], "-c") == 0))
    {
        int rc;
        const char *topic_name = argv[2];
        printf("Client subscribe to topic [%s]\n", topic_name);

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
    }

    if (alg_sdk_init_client())
    {
        printf("Init Client Error!\n");
        exit(0);
    }
    /* Do something here. */
    for (int i = 0; i < ALG_SDK_MAX_CHANNEL; i++)
    {
        if (&g_camera_disp_thred[i] != NULL)
        {
            pthread_join(g_camera_disp_thred[i], NULL);
        }
    }
    alg_sdk_client_spin_on();
    printf("Stop Client.");

    return 0;
}
