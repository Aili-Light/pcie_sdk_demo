#include "alg_sdk/render.h"
//#include "alg_sdk/util.h"
//#include "alg_common/log.h"
#include "alg_common/basic_types.h"

#include <semaphore.h>
#include <pthread.h>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/video/video.h>

#include "jetson-utils/RingBuffer.h"
#include "jetson-utils/imageFormat.h"
#include "jetson-utils/cudaColorspace.h"
#include "jetson-utils/cudaMappedMemory.h"

#include <sys/prctl.h>

#define STREAMER_NUM_MAX 16

static sem_t empty[ALG_SDK_MAX_CHANNEL];
static sem_t full[ALG_SDK_MAX_CHANNEL];
static pthread_t strm[ALG_SDK_MAX_CHANNEL];
static pthread_mutex_t mutex[ALG_SDK_MAX_CHANNEL];
static pcie_item_queue_t item_queue_ch[STREAMER_NUM_MAX];// = NULL;

const char topic_image_head_t[ALG_SDK_HEAD_COMMON_TOPIC_NAME_LEN] = {ALG_SDK_TOPIC_NAME_IMAGE_DATA};
const char topic_dev_poc_head_t[ALG_SDK_HEAD_COMMON_TOPIC_NAME_LEN] = {ALG_SDK_TOPIC_NAME_POC_INFO};

typedef struct alg_sdk_stream_arg{
    void*                   arg;
    void*                   ch_id;
}alg_sdk_stream_arg_t;
alg_sdk_stream_arg_t stream_arg_t[ALG_SDK_MAX_CHANNEL];

#define ALG_SDK_RENDER_STRING_SIZE 256
typedef struct _GstElement GstElement;
typedef struct _GstBus GstBus;
typedef struct _GstCaps GstCaps;
typedef struct alg_sdk_gst_render {
    char    launch_str[ALG_SDK_RENDER_STRING_SIZE*4];
    char    protocol[ALG_SDK_RENDER_STRING_SIZE];
    char    host_ip[ALG_SDK_RENDER_STRING_SIZE];
    char    port_num[ALG_SDK_RENDER_STRING_SIZE];
    char    codec_type[ALG_SDK_RENDER_STRING_SIZE];
    char    cap_str[ALG_SDK_RENDER_STRING_SIZE];
    char    v4l2_device[ALG_SDK_RENDER_STRING_SIZE];

    int     width;
    int     height;
    int     frame_rate;
    int     format;
    int     ch_id;

    bool    need_data;
    bool    mStreaming;

    GstBus*     mBus;
    GstElement* mAppSrc;
    GstElement* mPipeline;
    GstCaps*    mBufferCaps;

    RingBuffer mBufferYUV;
}alg_sdk_gst_render_t;

static alg_sdk_gst_render_t gst_streamer[ALG_SDK_MAX_CHANNEL];

enum
{
    ALG_SDK_VIDEO_FORMAT_UNKOWN = 0,
    ALG_SDK_VIDEO_FORMAT_I420 = 2,       /* Planar YUY-420 format  */
    ALG_SDK_VIDEO_FORMAT_YV12,           /* Planar YUY-422 format  */
    ALG_SDK_VIDEO_FORMAT_YUY2 = 4,       /* Packed YUY-422 format (Y-U-Y-V-Y-U-Y-V) */
    ALG_SDK_VIDEO_FORMAT_RGB = 15,       /* RGB packed into 24 bits without padding (R-G-B-R-G-B) */
    ALG_SDK_VIDEO_FORMAT_RAW10 = 90,       /* Packed YUY-422 format (Y-U-Y-V-Y-U-Y-V) */
};

bool alg_sdk_stream_render_init()
{
    static bool gstreamer_initialized = false;

    if( gstreamer_initialized )
        return true;

    int argc = 0;
    //char* argv[] = { "none" };

    if( !gst_init_check(&argc, NULL, NULL) )
    {
        printf("failed to initialize gstreamer library with gst_init()\n");
        return false;
    }

    gstreamer_initialized = true;

    uint32_t ver[] = { 0, 0, 0, 0 };
    gst_version( &ver[0], &ver[1], &ver[2], &ver[3] );
    printf("initialized gstreamer, version %u.%u.%u.%u\n", ver[0], ver[1], ver[2], ver[3]);

    // debugging
    gst_debug_remove_log_function(gst_debug_log_default);

    if( true )
    {
//        gst_debug_add_log_function(rilog_debug_function, NULL, NULL);
        gst_debug_set_active(true);
        gst_debug_set_colored(false);
    }

    return true;
}

int alg_sdk_render_build_launch_str (void* _streamer)
{
    if( !_streamer )
        return 1;

    alg_sdk_gst_render_t* streamer = (alg_sdk_gst_render_t*)_streamer;

    /* Launch string */
    char app_src[256] = "";
    char cap_str[256];
    char prtcl_str[256];
    char codec_str[256];

    if (streamer->format == ALG_SDK_VIDEO_FORMAT_YUY2)
    {
        sprintf(app_src, "appsrc name=mysource_%02d is-live=true do-timestamp=true ! ", streamer->ch_id);
        strcat(streamer->launch_str, app_src);

        if (strcmp(streamer->codec_type, "raw") == 0)
        {
            /* codec is raw */
            strcpy(cap_str, "rawvideoparse use-sink-caps=true ! ");
    //        sprintf(cap_str, "rawvideoparse use-sink-caps=false width=%d height=%d framerate=%d/1 format=4 ! ",
    //                width, height, frame_rate);
            strcpy(codec_str, " videoconvert ! ");
        }
        else if (strcmp(streamer->codec_type, "h264") == 0)
        {
            /* codec is h264 */
            /* video/x-raw, width=640, height=480, framerate=30/1' ! videoconvert ! x264enc ! queue ! rtph264pay */
            sprintf(cap_str, "rawvideoparse use-sink-caps=true ! ");
            strcpy(codec_str, "videoconvert ! nvh264enc bitrate=2048 ! h264parse ! ");
        }
        else
        {
            printf("gst Create -- unsupported video codec (%s)\n", streamer->codec_type);
            printf("              supported video codec are:\n");
            printf("                 * raw\n");
            printf("                 * h264\n");

            return 1;
        }
        strcat(streamer->launch_str, cap_str);
        strcat(streamer->launch_str, codec_str);

        if (strcmp(streamer->protocol, "rtp") == 0)
        {
            /* streaming protocol is rtp */
            strcpy(prtcl_str, "");
            if (strcmp(streamer->codec_type, "raw") == 0)
            {
                strcat(prtcl_str, "rtpvrawpay ! queue ! udpsink host=");
            }
            else if(strcmp(streamer->codec_type, "h264") == 0)
            {
                strcat(prtcl_str, "rtph264pay ! queue ! udpsink host=");
            }
            strcat(prtcl_str, streamer->host_ip);
            strcat(prtcl_str, " port=");
            strcat(prtcl_str, streamer->port_num);
        }
        else if (strcmp(streamer->protocol, "v4l2") == 0)
        {
            strcpy(prtcl_str, "");
            // strcat(prtcl_str, "qtmux ! filesink location=test_video.h264.mp4");
            strcat(prtcl_str, "v4l2sink device=");
            strcat(prtcl_str, streamer->v4l2_device);
        }
        else
        {
            printf("gst Create -- unsupported protocol (%s)\n", streamer->protocol);
            printf("              supported protocol are:\n");
            printf("                 * rtp\n");
            printf("                 * v4l2\n");

            return 1;
        }
        strcat(streamer->launch_str, prtcl_str);

        printf("launch string = %s\n", streamer->launch_str);
    }
    else
    {
        printf("gst Create -- unsupported format (%d)\n", streamer->format);
        printf("              supported format are:\n");
        printf("                 * ALG_SDK_VIDEO_FORMAT_YUY2\n");
    }

    return 0;
}

// onNeedData
void alg_sdk_render_on_need_data( GstElement* pipeline, guint size, gpointer user_data )
{
//    printf("ALG-Streamer -- appsrc requesting data (%u bytes)\n", size);

    if( !user_data )
        return;

    alg_sdk_gst_render_t* enc = (alg_sdk_gst_render_t*)user_data;
    enc->need_data  = true;
}

// onEnoughData
void alg_sdk_render_on_enough_data( GstElement* pipeline, gpointer user_data )
{
//    printf("ALG-Streamer -- appsrc signalling enough data\n");

    if( !user_data )
        return;

    alg_sdk_gst_render_t* enc = (alg_sdk_gst_render_t*)user_data;
    enc->need_data  = false;
}

// checkMsgBus
void alg_sdk_render_check_msg_bus (void* _streamer)
{
    if( !_streamer )
        return;

    alg_sdk_gst_render_t* streamer = (alg_sdk_gst_render_t*)_streamer;

    while(true)
    {
        GstMessage* msg = gst_bus_pop(streamer->mBus);

        if( !msg )
            break;

//        gst_message_print(streamer->mBus, msg, NULL);
        gst_message_unref(msg);
    }
}


bool alg_sdk_open_render (void* _streamer)
{
    if( !_streamer )
        return false;

    alg_sdk_gst_render_t* streamer = (alg_sdk_gst_render_t*)_streamer;

    if( streamer->mStreaming )
        return true;

    // transition pipline to STATE_PLAYING
    printf("ALG-Streamer -- starting pipeline, transitioning to GST_STATE_PLAYING\n");

    const GstStateChangeReturn result = gst_element_set_state(streamer->mPipeline, GST_STATE_PLAYING);

    if( result == GST_STATE_CHANGE_ASYNC )
    {
#if 0
        GstMessage* asyncMsg = gst_bus_timed_pop_filtered(mBus, 5 * GST_SECOND,
                                  (GstMessageType)(GST_MESSAGE_ASYNC_DONE|GST_MESSAGE_ERROR));

        if( asyncMsg != NULL )
        {
            gst_message_print(mBus, asyncMsg, this);
            gst_message_unref(asyncMsg);
        }
        else
            printf(LOG_GSTREAMER "gstEncoder -- NULL message after transitioning pipeline to PLAYING...\n");
#endif
    }
    else if( result != GST_STATE_CHANGE_SUCCESS )
    {
        printf("ALG-Streamer -- failed to set pipeline state to PLAYING (error %u)\n", result);
        return false;
    }

    alg_sdk_render_check_msg_bus(streamer);
    usleep(100 * 1000);
    alg_sdk_render_check_msg_bus(streamer);

    streamer->mStreaming = true;
    return true;
}

// Close
void alg_sdk_close_render(void* _streamer)
{
    if( !_streamer )
        return;

    alg_sdk_gst_render_t* streamer = (alg_sdk_gst_render_t*)_streamer;

    if( !streamer->mStreaming )
        return;

    // send EOS
    streamer->need_data = false;

    printf("ALG-Streamer -- shutting down pipeline, sending EOS\n");
    GstFlowReturn eos_result = gst_app_src_end_of_stream(GST_APP_SRC(streamer->mAppSrc));

    if( eos_result != 0 )
        printf("ALG-Streamer -- failed sending appsrc EOS (result %u)\n", eos_result);

    sleep(1);

    // stop pipeline
    printf("ALG-Streamer -- transitioning pipeline to GST_STATE_NULL\n");

    const GstStateChangeReturn result = gst_element_set_state(streamer->mPipeline, GST_STATE_NULL);

    if( result != GST_STATE_CHANGE_SUCCESS )
        printf("ALG-Streamer -- failed to set pipeline state to NULL (error %u)\n", result);

    sleep(1);
    alg_sdk_render_check_msg_bus(streamer);
    streamer->mStreaming = false;

    if( streamer->mAppSrc != NULL )
    {
        gst_object_unref(streamer->mAppSrc);
        streamer->mAppSrc = NULL;
    }

    if( streamer->mBus != NULL )
    {
        gst_object_unref(streamer->mBus);
        streamer->mBus = NULL;
    }

    if( streamer->mPipeline != NULL )
    {
        gst_object_unref(streamer->mPipeline);
        streamer->mPipeline = NULL;
    }

    printf("ALG-Streamer -- pipeline stopped\n");
}


int alg_sdk_render_build_cap_str (void* _streamer)
{
    if( !_streamer )
        return 1;

    alg_sdk_gst_render_t* streamer = (alg_sdk_gst_render_t*)_streamer;

//    sprintf(streamer->cap_str, "video/x-raw, width=%d, height=%d, format=(string)YUY2, framerate=%d/1",
//            streamer->width,streamer->height,streamer->frame_rate
//            );

    if(strcmp(streamer->codec_type, "raw") == 0)
    {
        sprintf(streamer->cap_str, "video/x-raw,width=(int)%d,height=(int)%d,framerate=(fraction)30/1,format=(string)YUY2",
            streamer->width,streamer->height);
    }
    else if (strcmp(streamer->codec_type, "h264") == 0)
    {
        sprintf(streamer->cap_str, "video/x-raw,width=(int)%d,height=(int)%d,framerate=(fraction)30/1,format=(string)I420",
            streamer->width,streamer->height);        
    }
    else
    {
        printf( "Stream Render -- unsupported codec type (%s)\n", streamer->codec_type);
		printf( "                        supported formats are:\n");
		printf( "                            * raw\n");		
		printf( "                            * h264\n");

        return 1;
    }

    return 0;
}


// encode image
int alg_sdk_render_encode_image ( void* buffer, int width, int height, void* _streamer)
{
    if( !buffer || width == 0 || height == 0 || !_streamer )
        return 1;

    alg_sdk_gst_render_t* streamer = (alg_sdk_gst_render_t*)_streamer;

    if (streamer->width != width)
    {
        streamer->width = width;
    }
    if (streamer->height != height)
    {
        streamer->height = height;
    }

    size_t size = width * height * 2;

    // confirm the stream is open
    if( !streamer->mStreaming )
    {
        printf("-----------------Open streamer------------------------\n");
        if( !alg_sdk_open_render(streamer) )
            return 1;
    }

    // check to see if data can be accepted
    if( !streamer->need_data )
    {
        printf("ALG-Streamer -- pipeline full, skipping frame (%zu bytes)\n", size);
        return 0;
    }

#if 1
    // construct the buffer caps for this size image
    if( !streamer->mBufferCaps )
    {
        /* cap string never initialized (for this resolution) */
        if( alg_sdk_render_build_cap_str(streamer) )
        {
            printf( "ALG-Streamer -- failed to build caps string\n");
            return 1;
        }
        printf("ALG-Gstream new cap str [ch:%d]: ", streamer->ch_id);
        printf("%s\n", streamer->cap_str);

        streamer->mBufferCaps = gst_caps_from_string(streamer->cap_str);
        if( !streamer->mBufferCaps )
        {
            printf("ALG-Streamer -- failed to parse caps from string:\n");
            printf("   %s\n", streamer->cap_str);
            return 1;
        }
        gst_app_src_set_caps(GST_APP_SRC(streamer->mAppSrc), streamer->mBufferCaps);
    }
#endif
    // allocate gstreamer buffer memory
    GstBuffer* gstBuffer = gst_buffer_new_allocate(NULL, size, NULL);

    // map the buffer for write access
    GstMapInfo map;

    if( gst_buffer_map(gstBuffer, &map, GST_MAP_WRITE) )
    {
        if( map.size != size )
        {
            printf("ALG-Streamer -- gst_buffer_map() size mismatch, got %zu bytes, expected %zu bytes\n", map.size, size);
            gst_buffer_unref(gstBuffer);
            return 1;
        }

        memcpy(map.data, buffer, size);
        gst_buffer_unmap(gstBuffer, &map);
    }
    else
    {
        printf("ALG-Streamer -- failed to map gstreamer buffer memory (%zu bytes)\n", size);
        gst_buffer_unref(gstBuffer);
        return 1;
    }

    // queue buffer to gstreamer
    GstFlowReturn ret;
    g_signal_emit_by_name(streamer->mAppSrc, "push-buffer", gstBuffer, &ret);
    gst_buffer_unref(gstBuffer);

    if( ret != 0 )
        printf("ALG-Streamer -- appsrc pushed buffer abnormally (result %u)\n", ret);

    alg_sdk_render_check_msg_bus(streamer);
    return 0;
}

// encode image
int alg_sdk_render_encode_image_v2 ( void* buffer, size_t img_size, void* _streamer)
{
    if( !buffer || img_size == 0 || !_streamer )
        return 1;

    alg_sdk_gst_render_t* streamer = (alg_sdk_gst_render_t*)_streamer;

    // confirm the stream is open
    if( !streamer->mStreaming )
    {
        printf("-----------------Open streamer------------------------\n");
        if( !alg_sdk_open_render(streamer) )
            return 1;
    }

    // check to see if data can be accepted
    if( !streamer->need_data )
    {
        printf("ALG-Streamer -- pipeline full, skipping frame (%zu bytes)\n", img_size);
        return 0;
    }

#if 1
    // construct the buffer caps for this size image
    if( !streamer->mBufferCaps )
    {
        /* cap string never initialized (for this resolution) */
        if( alg_sdk_render_build_cap_str(streamer) )
        {
            printf( "ALG-Streamer -- failed to build caps string\n");
            return 1;
        }
        printf("ALG-Gstream new cap str [ch:%d]: ", streamer->ch_id);
        printf("%s\n", streamer->cap_str);

        streamer->mBufferCaps = gst_caps_from_string(streamer->cap_str);
        if( !streamer->mBufferCaps )
        {
            printf("ALG-Streamer -- failed to parse caps from string:\n");
            printf("   %s\n", streamer->cap_str);
            return 1;
        }
        gst_app_src_set_caps(GST_APP_SRC(streamer->mAppSrc), streamer->mBufferCaps);
    }
#endif
    // allocate gstreamer buffer memory
    GstBuffer* gstBuffer = gst_buffer_new_allocate(NULL, img_size, NULL);

    // map the buffer for write access
    GstMapInfo map;

    if( gst_buffer_map(gstBuffer, &map, GST_MAP_WRITE) )
    {
        if( map.size != img_size )
        {
            printf("ALG-Streamer -- gst_buffer_map() size mismatch, got %zu bytes, expected %zu bytes\n", map.size, img_size);
            gst_buffer_unref(gstBuffer);
            return 1;
        }

        memcpy(map.data, buffer, img_size);
        gst_buffer_unmap(gstBuffer, &map);
    }
    else
    {
        printf("ALG-Streamer -- failed to map gstreamer buffer memory (%zu bytes)\n", img_size);
        gst_buffer_unref(gstBuffer);
        return 1;
    }

    // queue buffer to gstreamer
    GstFlowReturn ret;
    g_signal_emit_by_name(streamer->mAppSrc, "push-buffer", gstBuffer, &ret);
    gst_buffer_unref(gstBuffer);

    if( ret != 0 )
        printf("ALG-Streamer -- appsrc pushed buffer abnormally (result %u)\n", ret);

    alg_sdk_render_check_msg_bus(streamer);
    return 0;
}

// render image to i420 format (by CUDA)
int alg_sdk_render (void *buffer,  int width, int height, int video_format, void* _streamer)
{
    if( !buffer || width == 0 || height == 0 || !_streamer )
    {
        printf("Stream Render failed : buffer is empty!\n");
        return 1;
    }
    alg_sdk_gst_render_t* streamer = (alg_sdk_gst_render_t*)_streamer;

    if (streamer->width != width)
    {
        streamer->width = width;
    }
    if (streamer->height != height)
    {
        streamer->height = height;
    }

    imageFormat input_format;
    size_t      input_img_size;

    if (video_format == ALG_SDK_VIDEO_FORMAT_YUY2)
    {
        input_format = IMAGE_YUY2;
        input_img_size = imageFormatSize(input_format, width, height);
    }
    else if (video_format == ALG_SDK_VIDEO_FORMAT_RAW10)
    {
        // TODO
    }
    else
    {
        printf( "Stream Render -- unsupported input video format (%d)\n", video_format);
		printf( "                        supported formats are:\n");
		printf( "                            * yuy2\n");		
		printf( "                            * raw10\n");

        return 1;
    }

    int ret;

    if (strcmp(streamer->codec_type,"raw") == 0)
    {
        ret = alg_sdk_render_encode_image_v2(buffer, input_img_size, streamer);
        return ret;
    }
    else if (strcmp(streamer->codec_type,"h264") == 0)
    {
        const size_t i420Size = imageFormatSize(IMAGE_I420, width, height);

        if( !streamer->mBufferYUV.Alloc(2, i420Size, RingBuffer::ZeroCopy) )
        {
            printf("Stream Render -- failed to allocate buffers (%zu bytes each)\n", i420Size);
            // enc_success = false;
            return 1;
        }

        void* image_yuy2_gpu = NULL;
        if( !cudaAllocMapped(&image_yuy2_gpu, input_img_size) )
        {
            printf( "Stream Render -- failed to allocate %zu bytes for image! \n", input_img_size);
            return 1;
        }
        memcpy(image_yuy2_gpu, buffer, input_img_size);

        void* nextYUV = streamer->mBufferYUV.Next(RingBuffer::Write);

        if( CUDA_FAILED(cudaConvertColor(image_yuy2_gpu, input_format, nextYUV, IMAGE_I420, width, height)) )
        {
            printf( "Stream Render -- unsupported convert image format (%d)\n", input_format);
            printf( "                        supported formats are:\n");
            printf( "                            * yuy2 -> i420\n");		
            printf( "                            * yuyv -> i420\n");		

            return 1;
        }

        CUDA(cudaDeviceSynchronize());	// TODO replace with cudaStream?
        CUDA(cudaFreeHost(image_yuy2_gpu));

        ret = alg_sdk_render_encode_image_v2(nextYUV, i420Size, streamer);
        return ret;
    }
    else
    {
        printf( "Stream Render -- unsupported codec type (%s)\n", streamer->codec_type);
		printf( "                        supported formats are:\n");
		printf( "                            * raw\n");		
		printf( "                            * h264\n");

        return 1; 
    }

    return 0;
}

void *alg_sdk_rendering (void *arg)
{
    // int fd = (intptr_t)arg;
    alg_sdk_stream_arg_t *t = (alg_sdk_stream_arg_t*)arg;
    int ch_id = (intptr_t)t->ch_id;
    alg_sdk_gst_render_t* dev = (alg_sdk_gst_render_t*)t->arg;

#ifndef __MINGW32__
    prctl(PR_SET_NAME, (unsigned long)"alg_sdk_rendering");
#endif

    /*  Main processing loop. */
    while(1)
    {
        sem_wait(&full[ch_id]);
        pthread_mutex_lock(&mutex[ch_id]);

        if(!alg_sdk_queue_item_is_empty(&item_queue_ch[ch_id]))
        {
            pcie_item_manager_t msg_pub;
            memset(&msg_pub, 0, sizeof(pcie_item_manager_t));
            alg_sdk_queue_item_pop(&item_queue_ch[ch_id], &msg_pub);
            pcie_image_info_meta_t* metadata = (pcie_image_info_meta_t*)msg_pub.metadata;
//            printf("[ch %d] width = %d, height = %d\n", ch_id, metadata->width, metadata->height);

            /* TODO : data type should come from metadata 
                      manually set data_type is not permannat way!
            */
            // int ret = alg_sdk_render_encode_image ((char*)msg_pub.payload, metadata->width, metadata->height, dev);
            int ret = alg_sdk_render((char*)msg_pub.payload, metadata->width, metadata->height, ALG_SDK_VIDEO_FORMAT_YUY2, dev);
            if (!ret)
            {
                alg_sdk_log(LOG_ERROR, "Failed to encode image to stream\n");
            }

            /* release the thread lock so push2q can work on */
            pthread_mutex_unlock(&mutex[ch_id]);
//            sem_post(&empty[ch_id]);
        }
        else
        {
            /* Do not forget to release thread lock
                otherwise push2q may be waiting forever
            */
            alg_sdk_log(LOG_DEBUG, "[Channel %d] Queue is empty. Do Not publish. \n", ch_id);
            pthread_mutex_unlock(&mutex[ch_id]);
//            sem_post(&empty[ch_id]);
        }
    }

//    nn_close (fd);
    // TODO close stream
    return NULL;
}

int alg_sdk_push2render(const void* msg, const int ch_id)
{
//    sem_wait(&empty[ch_id]);
    pthread_mutex_lock(&mutex[ch_id]);

    if(msg)
    {
        unsigned char crc8 = ((unsigned char*)msg)[130];
        // printf("%d\n", crc_array((unsigned char*)msg, 130));

        /*  Check Header Format*/
        if(crc8 == crc_array((unsigned char*)msg, 130))
        {
            char topic_name[ALG_SDK_HEAD_COMMON_TOPIC_NAME_LEN];
            memcpy(topic_name, (char*)msg+2, ALG_SDK_HEAD_COMMON_TOPIC_NAME_LEN);

            if(strncmp(topic_name, topic_image_head_t, strlen(topic_image_head_t)) == 0)
            {
                pcie_item_manager_t t;
                t.msg_head = 0x66; // message head
                t.common_head = ((pcie_image_data_t*)msg)->common_head;
//                t.image_info_meta = ((pcie_image_data_t*)msg)->image_info_meta;
                t.metadata = &((pcie_image_data_t*)msg)->image_info_meta;
                t.meta_len = sizeof(pcie_image_info_meta_t);
                t.payload = ((pcie_image_data_t*)msg)->payload;
                t.payload_len = (size_t)(((pcie_image_data_t*)msg)->image_info_meta.img_size);
                // pcie_image_data_t *t = (pcie_image_data_t*)msg;
                alg_sdk_queue_item_push(&item_queue_ch[ch_id], &t);
//                printf("push2q ch %d\n", ch_id);
            }
            else if(strncmp(topic_name, topic_dev_poc_head_t, strlen(topic_dev_poc_head_t)) == 0)
            {
                // printf("%s\n", topic_name);
                pcie_item_manager_t t;
                t.msg_head = 0x66; // message head
                t.common_head = ((pcie_poc_info_t*)msg)->common_head;
//                t.poc_info_meta = ((pcie_poc_info_t*)msg)->poc_info_meta;
                t.metadata = &((pcie_poc_info_t*)msg)->poc_info_meta;
                t.meta_len = sizeof(pcie_poc_info_t);
                t.payload_len = 0; // no payload
                alg_sdk_queue_item_push(&item_queue_ch[ch_id], &t);
            }
            else
            {
                alg_sdk_log(LOG_ERROR, "PUSH2Q Error : Topic unknown.");
            }
        }
        else
        {
            alg_sdk_log(LOG_ERROR, "CHECK HEADER FAILED! PUSH DATA IN WRONG FORMAT.");
        }
    }
    else
    {
        alg_sdk_log (LOG_ERROR, "PUSH DATA EMPTY!");
    }
    // usleep(100);
    pthread_mutex_unlock(&mutex[ch_id]);
    sem_post(&full[ch_id]);

    return 0;
}

int alg_sdk_render_parse_args(int argn, char** arg, void* _streamer, int ch_id)
{
    if( !_streamer )
        return 1;

    alg_sdk_gst_render_t* streamer = (alg_sdk_gst_render_t*)_streamer;

    /* parsing arguments */
    if (argn > 0)
    {
        for (int i=0; i<argn; i++)
        {
            if (strncmp(arg[i], "protocol", strlen("protocol")) == 0)
            {
                int off = strlen("protocol");;
                int len = strlen(arg[i]) - off - 1;
                memcpy(streamer->protocol, arg[i]+off+1, len);
            }
            else if (strncmp(arg[i], "host_ip", strlen("host_ip")) == 0)
            {
                int off = strlen("host_ip");;
                int len = strlen(arg[i]) - off - 1;
                memcpy(streamer->host_ip, arg[i]+off+1, len);
            }
            else if (strncmp(arg[i], "port_num", strlen("port_num")) == 0)
            {
                int off = strlen("port_num");;
                int len = strlen(arg[i]) - off - 1;
                char c_port_n[ALG_SDK_RENDER_STRING_SIZE] = "";
                memcpy(c_port_n, arg[i]+off+1, len);
                int port_number = atoi(c_port_n);
                port_number = port_number + ch_id;
                sprintf(c_port_n, "%d", port_number);
//                printf("port number = %s\n", c_port_n);
                strcpy(streamer->port_num, c_port_n);
//                memcpy(streamer->port_num, arg[i]+off+1, len);
            }
            else if (strncmp(arg[i], "codec_type", strlen("codec_type")) == 0)
            {
                int off = strlen("codec_type");;
                int len = strlen(arg[i]) - off - 1;
                memcpy(streamer->codec_type, arg[i]+off+1, len);
            }
            else if (strncmp(arg[i], "width", strlen("width")) == 0)
            {
                int off = strlen("width");;
                int len = strlen(arg[i]) - off - 1;
                char c_width[128];
                memcpy(c_width, arg[i]+off+1, len);
                streamer->width = atoi(c_width);
            }
            else if (strncmp(arg[i], "height", strlen("height")) == 0)
            {
                int off = strlen("height");;
                int len = strlen(arg[i]) - off - 1;
                char c_height[128];
                memcpy(c_height, arg[i]+off+1, len);
                streamer->height = atoi(c_height);
            }
            else if (strncmp(arg[i], "frame_rate", strlen("frame_rate")) == 0)
            {
                int off = strlen("frame_rate");;
                int len = strlen(arg[i]) - off - 1;
                char c_frame_rate[128];
                memcpy(c_frame_rate, arg[i]+off+1, len);
                streamer->frame_rate = atoi(c_frame_rate);
            }
            else if (strncmp(arg[i], "format", strlen("format")) == 0)
            {
                int off = strlen("format");;
                int len = strlen(arg[i]) - off - 1;
                char c_format[128];
                memcpy(c_format, arg[i]+off+1, len);
                if (strcmp(c_format, "I420") == 0)
                {
                    streamer->format = ALG_SDK_VIDEO_FORMAT_I420;
                }
                else if (strcmp(c_format, "YUY2") == 0)
                {
                    streamer->format = ALG_SDK_VIDEO_FORMAT_YUY2;
                }
                else if (strcmp(c_format, "RGB") == 0)
                {
                    streamer->format = ALG_SDK_VIDEO_FORMAT_RGB;
                }
                else
                {
                    streamer->format = ALG_SDK_VIDEO_FORMAT_UNKOWN;
                }
            }
            else if (strncmp(arg[i], "v4l2_device", strlen("v4l2_device")) == 0)
            {
                int off = strlen("v4l2_device=/dev/video");
                int len = strlen(arg[i]) - off;
                char c_device_n[ALG_SDK_RENDER_STRING_SIZE] = "";
                memcpy(c_device_n, arg[i]+off, len);
                // printf("len = %ld, port num=%d\n", strlen(c_device_n), atoi(c_device_n));
                int port_number = atoi(c_device_n);
                port_number = port_number + ch_id;
                sprintf(streamer->v4l2_device, "/dev/video%d", port_number);
                // printf("v4l2 device is : %s\n", streamer->v4l2_device);
            }
        }

        /* use default parameters */
        if (streamer->protocol[0]=='\0')
        {
            strcpy(streamer->protocol, "rtp");
        }

        if (streamer->host_ip[0]=='\0')
        {
            strcpy(streamer->host_ip, "127.0.0.1");
        }

        if (streamer->port_num[0]=='\0')
        {
            strcpy(streamer->port_num, "5000");
        }

        if (streamer->codec_type[0]=='\0')
        {
            strcpy(streamer->codec_type, "raw");
        }

        if (streamer->width==0)
        {
            streamer->width = 1920;
        }

        if (streamer->height==0)
        {
            streamer->height = 1080;
        }

        if (streamer->frame_rate==0)
        {
            streamer->frame_rate = 30;
        }

        if (streamer->format==0)
        {
            streamer->format = ALG_SDK_VIDEO_FORMAT_YUY2;
        }

        if (streamer->v4l2_device[0]=='\0')
        {
            strcpy(streamer->v4l2_device, "/dev/video0");
        }

        streamer->ch_id = ch_id;

        printf("protocol=[%s], host_ip=[%s], port_num=[%s], codec_type=[%s], width=[%d]"
               ", height=[%d], frame_rate=[%d], format=[%d], channel=[%d]\n",
               streamer->protocol,streamer->host_ip,
               streamer->port_num, streamer->codec_type,
               streamer->width, streamer->height,
               streamer->frame_rate, streamer->format,
               streamer->ch_id);
    }
    else
    {
        printf("Init Stream Failed ! Please give specific configuration. \n");
        return 1;
    }

    return 0;
}

int alg_sdk_render_init_pipeline(void* _streamer)
{
    if( !_streamer )
        return 1;

    alg_sdk_gst_render_t* streamer = (alg_sdk_gst_render_t*)_streamer;

    // initialize GStreamer libraries
    if( !alg_sdk_stream_render_init() )
    {
        printf("Failed to initialize gstreamer API\n");
        return 1;
    }

    if (alg_sdk_render_build_launch_str(streamer))
    {
        printf("Failed to build launch string! \n");
        return 1;
    }

    // create the pipeline
    GError* err = NULL;
    streamer->mPipeline = gst_parse_launch(streamer->launch_str, &err);

    if( err != NULL )
    {
        printf("ALG Streamer -- failed to create pipeline\n");
        printf("   (%s)\n", err->message);
        g_error_free(err);
        return 1;
    }

    GstPipeline* pipeline = GST_PIPELINE(streamer->mPipeline);

    if( !pipeline )
    {
        printf("ALG Streamer -- failed to cast GstElement into GstPipeline\n");
        return false;
    }

    // retrieve pipeline bus
    streamer->mBus = gst_pipeline_get_bus(pipeline);

    if( !streamer->mBus )
    {
        printf("ALG Streamer -- failed to retrieve GstBus from pipeline\n");
        return 1;
    }

    // get the appsrc element
    char source_name[256];
    sprintf(source_name, "mysource_%02d", streamer->ch_id);

    GstElement* appsrcElement = gst_bin_get_by_name(GST_BIN(pipeline), source_name);
    GstAppSrc* appsrc = GST_APP_SRC(appsrcElement);

    if( !appsrcElement || !appsrc )
    {
        printf("ALG Streamer -- failed to retrieve appsrc element from pipeline\n");
        return 1;
    }

    streamer->mAppSrc = appsrcElement;

    g_signal_connect(appsrcElement, "need-data", G_CALLBACK(alg_sdk_render_on_need_data), streamer);
    g_signal_connect(appsrcElement, "enough-data", G_CALLBACK(alg_sdk_render_on_enough_data), streamer);

    return 0;
}

int alg_sdk_init_render(int argn, char** arg)
{
    alg_sdk_log(LOG_DEBUG, "Start Streamer");

    /* Init Gstreamer  */
    for(int i = 0; i < STREAMER_NUM_MAX; i++)
    {
        alg_sdk_render_parse_args(argn, arg, &gst_streamer[i], i);
        alg_sdk_render_init_pipeline(&gst_streamer[i]);
        gst_streamer[i].mBufferYUV.SetThreaded(false);

        alg_sdk_queue_item_init(&item_queue_ch[i]);
    }

    /* Create thread */
    for (int i = 0; i < ALG_SDK_MAX_CHANNEL; i++)
    {
        alg_sdk_stream_arg_t arg = {(void *)(alg_sdk_gst_render_t*)&gst_streamer[i], (void *)(intptr_t)i};
        stream_arg_t[i] = arg;
    }

    memset (strm, 0, sizeof (strm));
    for (int i=0; i < STREAMER_NUM_MAX; i++)
    {
        pthread_mutex_init(&mutex[i], NULL);
        sem_init(&empty[i], 0, ALG_SDK_SEM_INIT_BUFFER_SIZE);
        sem_init(&full[i], 0, 0);

        int rc;
        rc = pthread_create(&strm[i], NULL, alg_sdk_rendering, (void *)(&stream_arg_t[i]));
        if (rc < 0)
        {
            alg_sdk_log (LOG_VERBOSE, "Server Thread Create Error: %s\n", strerror (rc));
            return 1;
        }
    }

    printf("ALG-Streamer -- Finish initialization\n");
    return 0;
}

int alg_sdk_start_render()
{
    for (int i=0; i<STREAMER_NUM_MAX; i++)
    {
        if (&strm[i] != NULL)
        {
            pthread_join(strm[i], NULL);
        }
    }

    return 0;
}

int alg_sdk_stop_render()
{
    for(int i = 0; i < STREAMER_NUM_MAX; i++)
    {
        alg_sdk_close_render(&gst_streamer[i]);
    }

    for (int i=0; i<ALG_SDK_MAX_CHANNEL; i++)
    {
        pthread_mutex_destroy(&mutex[i]);
        sem_destroy(&empty[i]);
        sem_destroy(&full[i]);
    }

    alg_sdk_log(LOG_VERBOSE, "Stream Thread Stopped.");

    return 1;
}
