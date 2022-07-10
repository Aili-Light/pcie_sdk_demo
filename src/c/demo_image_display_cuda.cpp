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
videoOutput* g_output_stream;
static pthread_mutex_t mutex;
static pthread_t video_stream;

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
    delete g_ring_buffer;
    SAFE_DELETE(g_display);
    SAFE_DELETE(g_output_stream);
    pthread_mutex_destroy(&mutex);

    /* Stop SDK */
    alg_sdk_stop_client();

    /* terminate program */
    exit(sig);
}


int image_color_convert(char* img_ptr, void** img_out, int img_width, int img_height, imageFormat format)
{
    if (format == IMAGE_RGBA32F)
    {
        void* img_in = NULL;
        size_t data_size = img_height * img_width;

        if( !cudaAllocMapped(&img_in, data_size * 2) )
        {
            printf(" failed to allocate %ld bytes for image\n", data_size);
            return 1;
        }
        memcpy(img_in, (char*)img_ptr, data_size * 2);

        if( !cudaAllocMapped((void**)img_out, data_size * 16) )
        {
            printf(" failed to allocate %ld bytes for image\n", data_size);
            return 1;
        }

        if( CUDA_FAILED(cudaConvertColor(img_in, IMAGE_YUYV, img_out, IMAGE_RGBA32F, img_width, img_height)) )
        {
            printf(" failed to convert image from %s to %s\n", imageFormatToStr(IMAGE_YUYV), imageFormatToStr(IMAGE_RGBA32F));
            return 1;
        }
        CUDA(cudaFreeHost(img_in));
    }
    return 0;
}

void convert2rgb(char* data, int w, int h, uint32_t frame_index)
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

    if( CUDA_FAILED(cudaConvertColor(mem_item->img_yuv2u, IMAGE_YUYV, mem_item->img_rgba32f, IMAGE_RGBA32F, w, h)) )
    {
        printf(" failed to convert image from %s to %s\n", imageFormatToStr(IMAGE_YUYV), imageFormatToStr(IMAGE_RGBA32F));
        return;
    }

//    printf("-------1 push = %f\n", ((float*)mem_item->img_rgba32f)[100]);

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
    convert2rgb((char*)msg->payload, msg->image_info_meta.width, msg->image_info_meta.height, msg->image_info_meta.frame_index);  // YUV422 type is CV_8U2
}

int usage()
{
    printf("usage: video-viewer [--help] input_URI [output_URI]\n\n");
    printf("View/output a video or image stream.\n");
    printf("See below for additional arguments that may not be shown above.\n\n");
    printf("positional arguments:\n");
    printf("    input_URI       resource URI of input stream  (see videoSource below)\n");
    printf("    output_URI      resource URI of output stream (see videoOutput below)\n\n");

    printf("%s", videoSource::Usage());
    printf("%s", videoOutput::Usage());
    printf("%s", Log::Usage());

    return 0;
}

void *streaming_thread (void *arg)
{
    int fd = (intptr_t)arg;
    printf("Channel %d\n", fd);

    while(1)
    {
        int ret = g_ring_buffer->receive_signal_frm_refresh();
        if(ret == 0)
        {
//            printf("get signal\n");
            pthread_mutex_lock(&mutex);
//            printf("-------2 tail = %d\n", g_ring_buffer->read_tail());

            host_memory_manager_t* mem_item = g_ring_buffer->get_tail();
//            printf("-------2 pop = %d\n", mem_item->header);

            printf("-------2 pop = %f\n", ((float*)mem_item->img_rgba32f)[10086]);
//            printf("-------2 width = %d, height = %d\n", mem_item->width, mem_item->height);


            g_display->RenderOnce((float*)mem_item->img_rgba32f, mem_item->width, mem_item->height);

//            // update status bar
            g_display->SetTitle("Image Display");

            // check if the user quit
            if( g_display->IsClosed() )
            {
                g_signal_recieved = true;
            }
//            g_output_stream->Render((float4*)mem_item->img_rgba32f, mem_item->width, mem_item->height, IMAGE_RGBA32F);
            pthread_mutex_unlock(&mutex);
        }
        usleep(100);
    }

    return NULL;
}

int main (int argc, char **argv)
{
    signal(SIGINT, int_handler);

    /*
     * parse command line
     */
    commandLine cmdLine(argc, argv);

    if( cmdLine.GetFlag("help") )
        return usage();

    /*
     * create output video stream
     */
    videoOptions options;
    options.width = 640;
    options.height = 480;
    options.resource.protocol = "file";
    options.resource.location = "myvideo.mp4";
    options.resource.extension = "mp4";
    options.codec = videoOptions::Codec(8);
    g_output_stream = videoOutput::Create(options);

    if( !g_output_stream )
        printf("video-viewer:  failed to create output stream\n");


    /*
     * create openGL window
     */

//    g_display = glDisplay::Create(options);
//    if( !g_display )
//    {
//        printf("camera-viewer:  failed to create openGL display\n");
//    }

    g_ring_buffer = new host_memory_ring_buffer();
    if(g_ring_buffer->init_ring_buffer(1920*1280))
    {
        printf("init ring buffer failed!\n");
        return 0;
    }
    pthread_mutex_init(&mutex, NULL);

    int rc;
    const char* topic_name = "/image_data/stream/00";
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

    while(1)
    {
        int ret = g_ring_buffer->receive_signal_frm_refresh();
        if(ret == 0)
        {
            pthread_mutex_lock(&mutex);

            host_memory_manager_t* mem_item_s = g_ring_buffer->get_tail();

            g_output_stream->Render((float*)mem_item_s->img_rgba32f, mem_item_s->width, mem_item_s->height, IMAGE_RGBA32F);

            // update status bar
            g_output_stream->SetStatus("Image Display");

            // check if the user quit
            if( !g_output_stream->IsStreaming() )
                g_signal_recieved = true;

//            g_display->RenderOnce((float*)mem_item_s->img_rgba32f, mem_item_s->width, mem_item_s->height);

//    //            // update status bar
//            g_display->SetTitle("Image Display");

//            // check if the user quit
//            if( g_display->IsClosed() )
//            {
//                g_signal_recieved = true;
//            }
            pthread_mutex_unlock(&mutex);

        }
//        usleep(100000);
    }


    // Never reach this line :
    alg_sdk_client_spin_on();

    return 0;
}
