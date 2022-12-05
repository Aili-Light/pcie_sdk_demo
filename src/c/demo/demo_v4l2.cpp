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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "alg_common/basic_types.h"
#include "alg_camera/alg_camera.h"
bool g_signal_recieved = false;
AlgCamera g_alg_camera;

struct v4l2_dev v4l2loop_device = {
    .fd = -1,
    .path = (char *)"/dev/video100",
    .name = (char *)"v4l2loop_dev",
    .subdev_path = NULL,
    .out_type = {0},
    .buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
    .format = -1,
    .width = -1,
    .height = -1,
    .req_count = 2,
    .memory_type = V4L2_MEMORY_MMAP,
    .buffers = NULL,
    .sequence = 0,
    .timestamp = 0,
    .data_len = 0,
    .out_data = NULL,
    .buf_index = 0,
};

void int_handler(int sig)
{
    g_signal_recieved = True;
    stream_off(&v4l2loop_device);   // 8 关闭视频流
    close_device(&v4l2loop_device); // 9 释放内存关闭文件
    g_alg_camera.close_camera();
    /* terminate program */
    exit(sig);
}

int main(int argc, char *argv[])
{
    struct v4l2_dev *camdev = &v4l2loop_device;
    const char *v4l2_dev_name = 0;
    char name[40] = {0};

    if (argc > 1)
        camdev->path = argv[1];

    AlgCamera *alg_camera = &g_alg_camera;
    open_device(camdev); // 1 打开摄像头设备
    get_fmt(camdev);     // 2 查询出图格式
    get_capabilities(camdev);
    require_buf(camdev); // 3 申请缓冲区
    alloc_buf(camdev);   // 4 内存映射
    queue_buf(camdev);   // 5 将缓存帧加入队列
    stream_on(camdev);   // 6 开启视频流
    while (!g_signal_recieved)
    {
        if (!alg_camera->is_init())
        {
            printf("Init V4L2 Camera [%s]\n", camdev->path);
            const char *p_path = &camdev->path[11];
            char c_ch_id[2];
            strcpy(c_ch_id, p_path);
            int ch_id = atoi(c_ch_id);
            alg_camera->init_camera(ch_id, ALG_CAMERA_FLAG_SOURCE_V4L2);
        }
        capture_frame(camdev); // 7 取一帧数据
        alg_camera->capture_image(camdev);
        alg_camera->img_converter();
        alg_camera->render_image();

        if (alg_camera->is_closed())
            g_signal_recieved = true;
    }

    // snprintf(name, sizeof(name), "%s.%s", camdev->name, camdev->out_type);
    // save_picture(name, camdev->out_data, camdev->data_len, 1);

    return 0;
}
