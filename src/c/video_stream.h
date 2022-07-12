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

#ifndef __VIDEO_STREAM__
#define __VIDEO_STREAM__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>

#include "host_memory_manager.h"
#include "jetson-utils/videoOutput.h"

typedef struct video_stream_arg{
    void*                   arg;
    void*                   ch_id;
}video_stream_arg_t;

class VideoStream
{
public:
    VideoStream             (int ch_num);
    ~VideoStream            ();
    int                     create();
    int                     captureYUV(void* data, size_t width, size_t height, int ch_id);
    int                     start_streaming();
    int                     stop_streaming();

private:
    int                     channel_number;
    static void             *run (void*);
    bool                    *signal_recieved;

    videoOutput             **output_stream;
    host_memory_ring_buffer **ring_buffer; // each channel has 1 buffer
    pthread_t               *vstream;
    pthread_mutex_t         *mutex;
    video_stream_arg_t      *args;
};


#endif
