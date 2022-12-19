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
#ifndef __GST_CAMERA_H__
#define __GST_CAMERA_H__
#include "jetson-utils/RingBuffer.h"
#include "alg_common/basic_types.h"
#include "alg_sdk/stream.h"

class GstCamera 
{
public:
    GstCamera();
    ~GstCamera();

    /* Initialize Camera */
    int init_camera(int argn, char **arg, int _ch_id);

    /* End Camera */
    int camera_end();

    /* Start Streaming */
    int start_stream();

    /* Close Camera */
    int close_camera();

    /* Capture the Latest Frame */
    int capture_image(void *msg);

    /* Image Render & Display */
    void render_image();

    /* Return is camera init */    
    bool is_init();

    /* Return is camera streaming */    
    bool is_streaming();

    /* Calculate Frame Rate 
    */
    void frame_rate_monitor(const int frame_index);

private:

    /* camera initialized */
    bool b_init;

    /* Channel ID*/
    int ch_id;

    /* Image width */
    int width;

    /* Image height */
    int height;

    /* Image Data Type */
    int data_type;

    /* Image format */
    int format;

    /* Image Size */
    size_t img_size;

    /* Camera Type
     * Type=ALG_CAMERA_FLAG_SOURCE_PCIE : PCIE Image Data
     * Type=ALG_CAMERA_FLAG_SOURCE_V4L2 : V4L2 Source
     */
    int flag_src;

    /* flag is saving image */
    bool b_saving_image;

    /* Image Frame Index */
    uint32_t frame_index;

    /* Last Image Frame Index */
    uint32_t last_frame_idx;

    /* Frame Count */
    uint32_t frame_count;

    /* Image Timestamp */
    uint64_t timestamp;

    /* Last Image Timestamp */
    uint64_t last_timesmp;

    /* Frame Rate */
    float frame_rate;

    /* PCIE Image Data */
    pcie_image_data_t* pcie_image;

    /* GST Camera Device */
    alg_sdk_gst_stream_t* gst_camera;

    /* YUV Image */
    void* nextYUV;

    /* YUV Image Buffer */
    RingBuffer mBufferYUV;
};


#endif
