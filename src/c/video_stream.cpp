#include <unistd.h>
#include "video_stream.h"
#include "jetson-utils/videoOptions.h"

#define MAX_PAYLOAD_LEN 3840*2166

VideoStream::VideoStream(int ch_num) : channel_number(ch_num)
{
    printf("Initialize channel number [%d]\n", channel_number);

    output_stream = new videoOutput*[channel_number];
    ring_buffer = new host_memory_ring_buffer*[channel_number];
    signal_recieved = new bool[channel_number];
    vstream = new pthread_t[channel_number];
    mutex = new pthread_mutex_t[channel_number];
    args = new video_stream_arg_t[channel_number];
}

VideoStream::~VideoStream()
{
    for (int i=0; i < channel_number;i ++)
    {
        if(ring_buffer[i] != NULL)
        {
            delete ring_buffer[i];
        }
        pthread_mutex_destroy(&mutex[i]);
    }

    if(signal_recieved != NULL)
    {
        delete signal_recieved;
    }
    if(vstream != NULL)
    {
        delete vstream;
    }
    if(mutex != NULL)
    {
        delete mutex;
    }
    if(args != NULL)
    {
        delete args;
    }

    delete[] output_stream;
    delete[] ring_buffer;
}

void *VideoStream::run (void *args)
{
    video_stream_arg_t *t = (video_stream_arg_t*)args;
    VideoStream* vs = (VideoStream*)t->arg;
    int ch_id = (intptr_t)t->ch_id;

    videoOptions op = vs->output_stream[ch_id]->GetOptions();
    printf("Initialize thread id [%d] for stream [%s]\n", ch_id, op.resource.location.c_str());

    while(!vs->signal_recieved[ch_id])
     {
        int ret = vs->ring_buffer[ch_id]->receive_signal_frm_refresh();
        if(ret == 0)
        {
            pthread_mutex_lock(&(vs->mutex[ch_id]));
            host_memory_manager_t* mem_item_s = vs->ring_buffer[ch_id]->get_tail();
            vs->output_stream[ch_id]->Render(mem_item_s->img_yuv2u, mem_item_s->width, mem_item_s->height, IMAGE_YUYV);
            pthread_mutex_unlock(&(vs->mutex[ch_id]));

            // check if the user quit
            if( !vs->output_stream[ch_id]->IsStreaming() )
                vs->signal_recieved[ch_id] = true;

        }
    }

    return NULL;
}

int VideoStream::create()
{
    for (int i=0; i < channel_number;i ++)
    {
        char video_name[128] = {};
        sprintf(video_name, "myvideo_%02d", i);
        videoOptions option;
        option.resource.protocol = "file";
        option.resource.location = video_name;
        option.resource.extension = "mp4";
        option.codec = videoOptions::Codec(8);

        output_stream[i] = videoOutput::Create(option);
        ring_buffer[i] = new host_memory_ring_buffer();
        if(ring_buffer[i]->init_ring_buffer(MAX_PAYLOAD_LEN))
        {
            printf("init ring buffer failed!\n");
            return 1;
        }
        signal_recieved[i] = false;
    }

    for (int i = 0; i < channel_number; i++)
    {
        video_stream_arg_t arg = {(void *)this, (void *)(intptr_t)i};
        args[i] = arg;
    }

    for (int i = 0; i < channel_number; i++)
    {
        int rc;

        pthread_mutex_init(&mutex[i], NULL);
        rc = pthread_create(&vstream[i], NULL, &VideoStream::run, (void*)(&args[i]));
    }

    return 0;
}

int VideoStream::start_streaming()
{
    for (int i = 0; i < channel_number; i++)
    {
        if (&vstream[i] != NULL)
        {
            pthread_join(vstream[i], NULL);
        }
    }

    return 0;
}

int VideoStream::stop_streaming()
{
    /* receive stop streaming signal */
    for(int i=0; i< channel_number;i++)
    {
        signal_recieved[i] = true;
    }

    for(int i=0; i< channel_number;i++)
    {
        SAFE_DELETE(output_stream[i]);
        usleep(10000);
    }

    return 0;
}

int VideoStream::captureYUV(void* data, size_t width, size_t height, int ch_id)
{
    size_t data_size = width * height;

    pthread_mutex_lock(&mutex[ch_id]);
    host_memory_manager_t* mem_item = ring_buffer[ch_id]->get_head();
//    mem_item->header = frame_index;
    mem_item->height = height;
    mem_item->width = width;
    memcpy(mem_item->img_yuv2u, (char*)data, data_size * 2);
    pthread_mutex_unlock(&mutex[ch_id]);
    ring_buffer[ch_id]->send_signal_frm_refresh();

    return 0;
}
