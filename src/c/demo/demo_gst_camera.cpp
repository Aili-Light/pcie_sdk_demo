#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdint.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "alg_sdk/client.h"
#include "alg_common/basic_types.h"
#include "alg_camera/gst_camera.h"

#define ALG_SDK_IMAGE_DATA_BUFF_LEN 8
const char topic_image_head_d[ALG_SDK_HEAD_COMMON_TOPIC_NAME_LEN] = {ALG_SDK_TOPIC_NAME_IMAGE_DATA};

static pthread_t g_camera_disp_thred[ALG_SDK_MAX_CHANNEL];
static pthread_mutex_t mutex; //[ALG_SDK_MAX_CHANNEL];
static sem_t full[ALG_SDK_MAX_CHANNEL];
static uint64_t g_t_last[ALG_SDK_MAX_CHANNEL] = {0};
static uint32_t g_f_count[ALG_SDK_MAX_CHANNEL] = {0};
uint64_t g_tlast = 0;
bool g_signal_recieved = false;
bool g_send_eos = false;
bool b_streaming[ALG_SDK_MAX_CHANNEL];
uint32_t g_thread_num=0;

typedef struct camera_disp_args
{
    void *arg;
    void *ch_id;
} camera_disp_args_t;

camera_disp_args_t g_camera_args[ALG_SDK_MAX_CHANNEL];
GstCamera g_camera[ALG_SDK_MAX_CHANNEL];

void int_handler(int sig)
{
    alg_sdk_stop_client();
    g_signal_recieved = true;
    g_send_eos = true;
    printf("STOP SIGNAL\n");

    /* Waiting for EOS finish */
    sleep(2);

    // for (int i = 0; i < ALG_SDK_MAX_CHANNEL; i++)
    // {
    //     g_camera[i].camera_end();
    // }

    pthread_mutex_destroy(&mutex);
    for (int i = 0; i < g_thread_num; i++)
    {
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

    int ret;
    float fps = 0.0f;
    int ch_id = get_channel_id(msg);
    // uint64_t t1, t2, deltaT;
    // frame_monitor(ch_id, &fps, msg->image_info_meta.frame_index);
    // printf("[channel = %d], [frame = %d], [time %ld], [byte_0 = %d], [byte_end = %d]\n", ch_id,
    // msg->image_info_meta.frame_index,  msg->image_info_meta.timestamp, ((uint8_t*)msg->payload)[0], ((uint8_t*)msg->payload)[msg->image_info_meta.img_size - 1]);

    pthread_mutex_lock(&mutex);
    g_camera[ch_id].capture_image(msg);
    pthread_mutex_unlock(&mutex);
    sem_post(&full[ch_id]);
}

void *camera_display_thread(void *arg)
{
    camera_disp_args_t *t = (camera_disp_args_t *)arg;
    GstCamera *camera = (GstCamera *)t->arg;
    int ch_id = (intptr_t)t->ch_id;

    while (!g_signal_recieved)
    {
        sem_wait(&full[ch_id]);
        if (!camera->is_streaming())
        {
            printf("Start camera streaming [%d]\n", ch_id);
            camera->start_stream();
        }
        pthread_mutex_lock(&mutex);
        camera->render_image();
        pthread_mutex_unlock(&mutex);

        if (g_send_eos == true)
        {
            camera->close_camera();
        }
    }
}

int main(int argc, char **argv)
{
    signal(SIGINT, int_handler);

    printf("********Start GST Camera********\n");
    // initialize GStreamer libraries
    if (!alg_sdk_stream_gstreamer_init())
    {
        printf("Failed to initialize gstreamer API\n");
        return 1;
    }

    /* Create thread */
    int rc;
    int thred_n=0;
    pthread_mutex_init(&mutex, NULL);
    memset(g_camera_disp_thred, 0, sizeof(g_camera_disp_thred));

    char appsrc_parse[4][1024];
    char *appsrc[4];
    appsrc[0] = &appsrc_parse[0][0];
    appsrc[1] = &appsrc_parse[1][0];
    appsrc[2] = &appsrc_parse[2][0];
    appsrc[3] = &appsrc_parse[3][0];

    if (argc > 1)
    {
        if (strcmp(argv[1], "display") == 0)
        {
            strcpy(&appsrc_parse[0][0], "protocol=display");
            strcpy(&appsrc_parse[1][0], "codec_type=raw");
            strcpy(&appsrc_parse[2][0], "have_sei=0");
        }
        else if (strcmp(argv[1], "file") == 0)
        {
            strcpy(&appsrc_parse[0][0], "protocol=file");
            strcpy(&appsrc_parse[1][0], "codec_type=h264");
            strcpy(&appsrc_parse[2][0], "have_sei=0");
        }
        else if (strcmp(argv[1], "file-sei") == 0)
        {
            strcpy(&appsrc_parse[0][0], "protocol=file");
            strcpy(&appsrc_parse[1][0], "codec_type=h264");
            strcpy(&appsrc_parse[2][0], "have_sei=1");
        }
        else if (strcmp(argv[1], "file-ntp") == 0)
        {
            strcpy(&appsrc_parse[0][0], "protocol=file");
            strcpy(&appsrc_parse[1][0], "codec_type=h264");
            strcpy(&appsrc_parse[2][0], "have_sei=3");
        }
    }
    else
    {
        printf("ERROR ! Parse wrong argument!\n");
        exit(0);
    }

    if ((argc > 2) && (strcmp(argv[2], "-all") == 0))
    {
        if (argc > 3)
        {
            strcpy(&appsrc_parse[3][0], argv[3]);
        }
        
        char image_topic_names[ALG_SDK_MAX_CHANNEL][256];
        for (int i = 0; i < ALG_SDK_MAX_CHANNEL; i++)
        {
            camera_disp_args_t arg = {(void *)(GstCamera *)&g_camera[i], (void *)(intptr_t)i};
            g_camera_args[thred_n] = arg;
            sem_init(&full[i], 0, 0);
            thred_n++;

            rc = g_camera[i].init_camera(4, &appsrc[0], i);
            if (rc)
            {
                printf("Setup Gst Camera Error!\n");
                exit(0);
            }

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
    else if ((argc > 3) && (strcmp(argv[2], "-c") == 0))
    {
        const char *topic_name = argv[3];
        printf("Client subscribe to topic [%s]\n", topic_name);
        const char *ptr_topic = (const char*)(topic_name+19);
        char c_ch_id[2];
        strcpy(c_ch_id, ptr_topic);
        int ch_id = atoi(c_ch_id);

        if (argc > 4)
        {
            strcpy(&appsrc_parse[3][0], argv[4]);
        }

        camera_disp_args_t arg = {(void *)(GstCamera *)&g_camera[ch_id], (void *)(intptr_t)ch_id};
        g_camera_args[0] = arg;
        sem_init(&full[ch_id], 0, 0);
        thred_n=1;
        
        rc = g_camera[ch_id].init_camera(4, &appsrc[0], ch_id);
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

    /* Init Image Receive Client */
    if (alg_sdk_init_client())
    {
        printf("Init Client Error!\n");
        exit(0);
    }

    /* Create Display Thread */
    g_thread_num = thred_n;
    for (int i = 0; i < g_thread_num; i++)
    {
        rc = pthread_create(&g_camera_disp_thred[i], NULL, camera_display_thread, (void *)(&g_camera_args[i]));
        if (rc < 0)
        {
            printf("Server Thread Create Error: %s\n", strerror(rc));
            exit(0);
        }
    }

    /* Code Do Not Run This After. */
    for (int i = 0; i < g_thread_num; i++)
    {
        if (&g_camera_disp_thred[i] != NULL)
        {
            pthread_join(g_camera_disp_thred[i], NULL);
        }
    }
    alg_sdk_client_spin_on();

    /* Never Reach Here. */
    printf("*********Stop GST Camera***********");

    return 0;
}
