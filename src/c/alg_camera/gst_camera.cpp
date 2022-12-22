/*
 The MIT License (MIT)

Copyright (c) 2022 Aili-Light. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "gst_camera.h"
#include "alg_common/log.h"
#if defined (WITH_CUDA)
#include "jetson-utils/cuda_impl.h"
#endif
alg_sdk_gst_stream_t gst_streamer[ALG_SDK_MAX_CHANNEL];

uint64_t milliseconds(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (((uint64_t)tv.tv_sec * 1000) + ((uint64_t)tv.tv_usec / 1000));
}

GstCamera::GstCamera() {}

GstCamera::~GstCamera() {}

int GstCamera::init_camera(int argn, char **arg, int _ch_id)
{
    ch_id = _ch_id;
    gst_camera = &gst_streamer[ch_id];

    int ret;
    ret = alg_sdk_stream_parse_args(argn, arg, gst_camera, ch_id);
    if (ret)
    {
        printf("Setup gst camera failed! [Code:%d]\n", ret);
        return ret;
    }

    ret = alg_sdk_stream_init_pipeline(gst_camera);
    if (ret)
    {
        printf("Setup gst pipeline failed! [Code:%d]\n", ret);
        return ret;
    }

    b_init = true;
    return 0;
}

int GstCamera::camera_end()
{
    int ret;
    ret = alg_sdk_stream_deinit_pipeline(gst_camera);
    if (ret)
    {
        printf("End gst camera failed! [Code:%d]\n", ret);
        return ret;
    }

    return 0;
}

int GstCamera::start_stream()
{
    bool ret;
    // confirm the stream is open
    if (!gst_camera->mStreaming)
    {
        printf( "-----------------Open streamer------------------------\n");
        ret = alg_sdk_open_streamer(gst_camera);
        if (ret == false)
        {
            printf("Init gst camera [CH:%d] failed! [Code:%d]\n", ch_id, ret);
            return ret;
        }
    }

    return 0;
}

int GstCamera::close_camera()
{
    if (gst_camera->mStreaming)
    {
        printf( "-----------------Close Camera------------------------\n");
        /* Close Camera */
        alg_sdk_close_streamer(gst_camera);
    }

    return 0;
}

/* Capture the Latest Frame */
int GstCamera::capture_image(void *msg)
{
    if (msg == NULL)
        return 1;

    if (!gst_camera->mStreaming)
        return 1;

    // if (this->flag_src == ALG_CAMERA_FLAG_SOURCE_PCIE)
    {
        /* Data Source From PCIE */
        this->pcie_image = (pcie_image_data_t *)msg;
        this->nextYUV = this->pcie_image->payload;
        this->data_type = this->pcie_image->image_info_meta.data_type;
        this->width = this->pcie_image->image_info_meta.width;
        this->height = this->pcie_image->image_info_meta.height;
        this->frame_index = this->pcie_image->image_info_meta.frame_index;
        this->timestamp = this->pcie_image->image_info_meta.timestamp;
        this->img_size = this->pcie_image->image_info_meta.img_size;
    }

    if (!this->mBufferYUV.Alloc(5, img_size, RingBuffer::ZeroCopy))
    {
        printf("1****GstCamera -- failed to allocate buffers (%zu bytes each)\n", img_size);
        return 1;
    }

    void *yuv_img = this->mBufferYUV.Next(RingBuffer::Write);
    if (yuv_img != NULL)
    {
#if defined (WITH_CUDA)
        cuda_memcpy_h2d(yuv_img, nextYUV, img_size);
#else
        memcpy(yuv_img, nextYUV, img_size);
#endif
    }

    frame_rate_monitor(frame_index);
    return 0;
}

/* Image Render & Display */
void GstCamera::render_image()
{
    if (!gst_camera->mStreaming)
        return;

    void *next_img = this->mBufferYUV.Peek(RingBuffer::Read);
    alg_sdk_stream_encode_image(next_img, width, height, gst_camera, data_type, frame_index, timestamp);
}

void GstCamera::frame_rate_monitor(const int frame_index)
{
    /* Current frame should be last frame +1,
        otherwise some frames may be lost.
    */
    if ((frame_index - this->last_frame_idx <= 1) || (this->last_frame_idx == 0))
    {
        this->last_frame_idx = frame_index;
    }
    else
    {
        this->last_frame_idx = frame_index;
    }

    uint64_t t_now = milliseconds();
    this->frame_count++;
    uint64_t delta_t = t_now - last_timesmp;
    if (delta_t > 1000) // for 1000 milliseconds
    {
        last_timesmp = t_now;
        frame_rate = (float)this->frame_count / delta_t * 1000.0f;
        printf("Frame Monitor : [Channel %d] [Index %d] [Frame Rate = %f]\n", ch_id, frame_index, frame_rate);
        this->frame_count = 0;
    }
}

bool GstCamera::is_init()
{
    return b_init;
}

bool GstCamera::is_streaming()
{
    return gst_camera->mStreaming;
}