#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>
#include <fstream>
#include <pthread.h>

#include "alg_sdk/client.h"
#include "alg_common/basic_types.h"

#include "jetson-utils/cudaColorspace.h"
#include "jetson-utils/cudaMappedMemory.h"
#include "jetson-utils/gstCamera.h"
#include "jetson-utils/glDisplay.h"
#include "jetson-utils/imageIO.h"
#include "jetson-utils/videoOutput.h"

#include "host_memory_manager.h"

using namespace std;
const char topic_image_head_d[ALG_SDK_HEAD_COMMON_TOPIC_NAME_LEN] = {ALG_SDK_TOPIC_NAME_IMAGE_DATA};

static uint64_t g_t_last[ALG_SDK_MAX_CHANNEL] = {0};
static int g_f_count[ALG_SDK_MAX_CHANNEL] = {0};
static uint32_t g_f_last[ALG_SDK_MAX_CHANNEL] = {0};
static glDisplay* g_display;
bool g_signal_recieved = false;
host_memory_ring_buffer *g_ring_buffer;
static pthread_mutex_t mutex;
videoOutput* output_stream;
//static pthread_t video_stream;

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
    g_signal_recieved = true;

    /* The End */
    SAFE_DELETE(g_display);
    SAFE_DELETE(output_stream);

    pthread_mutex_destroy(&mutex);
    delete g_ring_buffer;

    /* Stop SDK */
    alg_sdk_stop_client();

    /* terminate program */
    exit(sig);
}


void push2buffer(char* data, size_t w, size_t h, uint32_t frame_index)
{
    size_t data_size = w * h;

    pthread_mutex_lock(&mutex);
//    printf("-------1 head = %d\n", g_ring_buffer->read_head());
    host_memory_manager_t* mem_item = g_ring_buffer->get_head();
    mem_item->header = frame_index;
    mem_item->height = h;
    mem_item->width = w;
//    printf("-------1 push = %d\n", mem_item->header);
    memcpy(mem_item->img_yuv2u, (char*)data, data_size * 2);
    pthread_mutex_unlock(&mutex);
    g_ring_buffer->send_signal_frm_refresh();
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
    push2buffer((char*)msg->payload, msg->image_info_meta.width, msg->image_info_meta.height, msg->image_info_meta.frame_index);  // YUV422 type is CV_8U2
}

int init_buffer(void)
{
    g_ring_buffer = new host_memory_ring_buffer();
    if(g_ring_buffer->init_ring_buffer(ALG_SDK_PAYLOAD_LEN_MAX))
    {
        printf("init ring buffer failed!\n");
        return 1;
    }
    pthread_mutex_init(&mutex, NULL);

    return 0;
}

int set_video_type(videoOptions* option)
{
    option->resource.protocol = "file";
    option->resource.location = "myvideo.mp4";
    option->resource.extension = "mp4";
    option->codec = videoOptions::Codec(8);

    return 0;
}

int main (int argc, char **argv)
{
    signal(SIGINT, int_handler);

    if(init_buffer())
    {
        /* Init Buffer Failed! */
        printf("init ring buffer failed!\n");
        exit(1);
    }

    if ((argc == 3) && (strcmp (argv[1], "-video_stream") == 0))
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
        /*
         * create output video stream
         */
        videoOptions options;
        set_video_type(&options);
        output_stream = videoOutput::Create(options);

        if( !output_stream )
        {
            printf("video-viewer:  failed to create output stream\n");
            exit(0);
        }

        while(!g_signal_recieved)
        {
            int ret = g_ring_buffer->receive_signal_frm_refresh();
            if(ret == 0)
            {
                pthread_mutex_lock(&mutex);
                host_memory_manager_t* mem_item_s = g_ring_buffer->get_tail();
                output_stream->Render(mem_item_s->img_yuv2u, mem_item_s->width, mem_item_s->height, IMAGE_YUYV);
                pthread_mutex_unlock(&mutex);

                // check if the user quit
                if( !output_stream->IsStreaming() )
                    g_signal_recieved = true;

            }
            usleep(100);
        }
        alg_sdk_client_spin_on();
    }
    else if ((argc == 3) && (strcmp (argv[1], "-image_display") == 0))
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

        videoOptions op;
        op.resource.protocol = "display";

        output_stream = videoOutput::Create(op);

        if( !output_stream )
        {
            printf("video-viewer:  failed to create output stream\n");
            exit(0);
        }

        while(!g_signal_recieved)
        {
            int ret = g_ring_buffer->receive_signal_frm_refresh();
            if(ret == 0)
            {
                pthread_mutex_lock(&mutex);
                host_memory_manager_t* mem_item_s = g_ring_buffer->get_tail();
                output_stream->Render(mem_item_s->img_yuv2u, mem_item_s->width, mem_item_s->height, IMAGE_YUYV);
                pthread_mutex_unlock(&mutex);

                // update status bar
                char str[256];
                sprintf(str, "Video Viewer (%ux%u) | %.1f FPS", output_stream->GetWidth(), output_stream->GetHeight(), output_stream->GetFrameRate());
                output_stream->SetStatus(str);

                // check if the user quit
                if( !output_stream->IsStreaming() )
                    g_signal_recieved = true;

            }
            usleep(100);
        }

        // Never reach this line :
        alg_sdk_client_spin_on();
    }






    return 0;
}
