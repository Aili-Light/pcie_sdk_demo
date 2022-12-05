#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "alg_common/basic_types.h"
#include "alg_camera/v4l2_camera.h"

struct v4l2_dev v4l2loop_device = {
    .fd = -1,
    .path = (char*)"/dev/video100",
    .name = (char*)"v4l2loop_dev",
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

int main (int argc, char *argv[])
{
    struct v4l2_dev *camdev = &v4l2loop_device;
    const char *v4l2_dev_name = 0;
    char name[40] = {0};

    if (argc > 1)
        camdev->path = argv[1];

    open_device(camdev); // 1 打开摄像头设备
    get_fmt(camdev);     // 2 查询出图格式
    get_capabilities(camdev);
    require_buf(camdev);   // 3 申请缓冲区
    alloc_buf(camdev);     // 4 内存映射
    queue_buf(camdev);     // 5 将缓存帧加入队列
    stream_on(camdev);     // 6 开启视频流
    capture_frame(camdev, -1); // 7 取一帧数据
    snprintf(name, sizeof(name), "%s.%s", camdev->name, camdev->out_type);
    save_picture(name, camdev->out_data, camdev->data_len, 1);

    stream_off(camdev);   // 8 关闭视频流
    close_device(camdev); // 9 释放内存关闭文件
    return 0;
}
