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

#ifndef __HOST_MEMORY_MANAGER_H__
#define __HOST_MEMORY_MANAGER_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <semaphore.h>
#include <pthread.h>

#define RING_BUFFER_SIZE 16

typedef struct host_memory_manager{
    int                     header;
    size_t                  height;
    size_t                  width;
    void                    *img_yuv2u;
    void                    *img_rgba32f;
}host_memory_manager_t;

class host_memory_ring_buffer
{
public:
    host_memory_ring_buffer ();
    ~host_memory_ring_buffer();
    int                     init_ring_buffer(size_t data_size);
    int                     deinit_ring_buffer();
    int                     push(host_memory_manager_t* t);
    int                     pop(host_memory_manager_t* t);
    int                     send_signal_frm_refresh();
    int                     receive_signal_frm_refresh();

    host_memory_manager_t*  get_head();
    host_memory_manager_t*  get_tail();
    int                     read_head();
    int                     read_tail();

private:
    void                    move_on(int *v);
    int                     is_empty();
    host_memory_manager_t   buffer[RING_BUFFER_SIZE];
    int                     head;
    int                     tail;
    sem_t                   sem;
};


#endif
