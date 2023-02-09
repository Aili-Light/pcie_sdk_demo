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
#include <fstream>
#include "alg_camera.h"
#include "alg_common/log.h"
#include "jetson-utils/cuda_impl.h"

// DWORD bfType;//固定为0x4d42;
//     DWORD bfSize; //文件大小
//     WORD bfReserved1; //保留字，不考虑
//     WORD bfReserved2; //保留字，同上
//     DWORD bfOffBits; //实际位图数据的偏移字节数，即前三个部分长度之和
// } BITMAPFILEHEADER;

typedef struct
{
    // unsigned short	bfType;	// 2B 文件类型,BM,BA,CT,CP,IC,PT
    unsigned int bfSize;        // 4B 位图文件大小，单位为B
    unsigned short bfReserved1; // 2B 保留
    unsigned short bfReserved2; // 2B 保留
    unsigned int bfOffBits;     // 4B 文件头开始到图像实际数据之间的偏移量，单位为B
} BmpFileHeader;

typedef struct
{
    unsigned int biSize;        // 4B 数据结构所需要的字节大小
    int biWidth;                // 4B 图像宽度
    int biHeight;               // 4B 图像高度
    unsigned short biPlane;     // 2B 颜色平面数
    unsigned short biBitCount;  // 2B 图像位深度：1，4，8，16，24，32
    unsigned int biCompression; // 4B 图像数据压缩类型：0(BI_RGB,不压缩),1(BI_RLE8，8比特游程编码),2(BI_RLE4，4比特游程编码),3(BI_BITFIELDS，比特域),4(BI_JPEG),5(BI_PNG)
    unsigned int biSizeImage;   // 4B 图像大小，用BI_RGB格式时，可设置为0.
    int biXPelsPerMeter;        // 4B 水平分辨率
    int biYPelsPerMeter;        // 4B 垂直分辨率
    unsigned int biClrUsed;     // 4B 颜色索引数
    unsigned int biClrImport;   // 4B 对图像显示有重要影响的颜色索引数，0表示都重要
} BmpInfoHeader;

enum BMPImageFormat
{
    BMP_IMAGE_BINARY = 0x0001, // 二值图像
    BMP_IMAGE_GREY = 0x0008,   // 灰度图像
    BMP_IMAGE_RGB = 0x0018,    // 彩色RGB图像
    BMP_IMAGE_BGR = 0x0019,    // 彩色BGR图像
    BMP_IMAGE_RGBA = 0x0020,   // 彩色RGBA图像
    BMP_IMAGE_BGRA = 0x0021    // 彩色BGRA图像
};

#define SAFE_DELETE(x) \
    if (x != NULL)     \
    {                  \
        delete x;      \
        x = NULL;      \
    }

uint64_t milliseconds(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (((uint64_t)tv.tv_sec * 1000) + ((uint64_t)tv.tv_usec / 1000));
}

AlgCamera::AlgCamera()
{
    b_init = false;
}
AlgCamera::~AlgCamera() {}

int AlgCamera::init_camera(int _ch_id, int flag)
{
    this->display = glDisplay::Create();

    if (!this->display)
    {
        alg_sdk_log(LOG_ERROR, "camera-viewer:  failed to create openGL display\n");
        return 1;
    }

    this->height = 0;
    this->width = 0;
    this->b_init = true;
    this->ch_id = _ch_id;
    this->flag_src = flag;
    this->frame_count = 0;
    this->b_saving_image = false;
    return 0;
}

int AlgCamera::close_camera()
{
    if (this->display)
        SAFE_DELETE(this->display);

    return 0;
}

int AlgCamera::capture_image(void *msg)
{
    if (msg == NULL)
        return 1;

    if (this->flag_src == ALG_CAMERA_FLAG_SOURCE_PCIE)
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
        // printf("[channel = %d], [frame = %d], [time %ld], [byte_0 = %d], [byte_end = %d]\n", this->ch_id,
        // pcie_image->image_info_meta.frame_index,  pcie_image->image_info_meta.timestamp, ((uint8_t*)this->nextYUV)[0], ((uint8_t*)this->nextYUV)[pcie_image->image_info_meta.img_size - 1]);
    }
    else if (this->flag_src == ALG_CAMERA_FLAG_SOURCE_V4L2)
    {
        v4l2_dev *v4l2_device = (v4l2_dev *)msg;
        switch (v4l2_device->format)
        {
        case V4L2_PIX_FMT_YVYU:
            this->data_type = ALG_SDK_MIPI_DATA_TYPE_YVYU;
            break;
        case V4L2_PIX_FMT_YUYV:
            this->data_type = ALG_SDK_MIPI_DATA_TYPE_YUYV;
            break;
        case V4L2_PIX_FMT_UYVY:
            this->data_type = ALG_SDK_MIPI_DATA_TYPE_UYVY;
            break;
        case V4L2_PIX_FMT_VYUY:
            this->data_type = ALG_SDK_MIPI_DATA_TYPE_VYUY;
            break;
        default:
            this->data_type = ALG_SDK_MIPI_DATA_TYPE_DEFAULT;
            break;
        }
        this->nextYUV = v4l2_device->out_data;
        this->frame_index = v4l2_device->sequence;
        this->timestamp = v4l2_device->timestamp;
        this->width = v4l2_device->width;
        this->height = v4l2_device->height;
        this->img_size = v4l2_device->buffers[v4l2_device->buf_index].length;
    }
    else
    {
        return 1;
    }

    switch (this->data_type)
    {
    case ALG_SDK_MIPI_DATA_TYPE_DEFAULT:
        this->format = ALG_SDK_VIDEO_FORMAT_YUY2;
        break;
    case ALG_SDK_MIPI_DATA_TYPE_YUYV:
        this->format = ALG_SDK_VIDEO_FORMAT_YUY2;
        break;
    case ALG_SDK_MIPI_DATA_TYPE_YVYU:
        this->format = ALG_SDK_VIDEO_FORMAT_YVYU;
        break;
    case ALG_SDK_MIPI_DATA_TYPE_UYVY:
        this->format = ALG_SDK_VIDEO_FORMAT_UYVY;
        break;
    case ALG_SDK_MIPI_DATA_TYPE_VYUY:
        this->format = ALG_SDK_VIDEO_FORMAT_VYUY;
        break;
    case ALG_SDK_MIPI_DATA_TYPE_RAW10:
        this->format = ALG_SDK_VIDEO_FORMAT_RAW10;
        break;
    case ALG_SDK_MIPI_DATA_TYPE_RAW12:
        this->format = ALG_SDK_VIDEO_FORMAT_RAW12;
        break;
    case ALG_SDK_MIPI_DATA_TYPE_Y2UV:
        this->format = ALG_SDK_VIDEO_FORMAT_Y2UV;
        break;
    case ALG_SDK_MIPI_DATA_TYPE_Y2VU:
        this->format = ALG_SDK_VIDEO_FORMAT_Y2VU;
        break;
    case ALG_SDK_MIPI_DATA_TYPE_UVY2:
        this->format = ALG_SDK_VIDEO_FORMAT_UVY2;
        break;
    case ALG_SDK_MIPI_DATA_TYPE_VUY2:
        this->format = ALG_SDK_VIDEO_FORMAT_VUY2;
        break;
    default:
        this->format = ALG_SDK_VIDEO_FORMAT_UNKOWN;
        break;
    }

    if (this->format == ALG_SDK_VIDEO_FORMAT_YUY2 || this->format == ALG_SDK_VIDEO_FORMAT_YVYU ||
        this->format == ALG_SDK_VIDEO_FORMAT_UYVY || this->format == ALG_SDK_VIDEO_FORMAT_VYUY ||
        this->format == ALG_SDK_VIDEO_FORMAT_Y2UV || this->format == ALG_SDK_VIDEO_FORMAT_Y2VU ||
        this->format == ALG_SDK_VIDEO_FORMAT_UVY2 || this->format == ALG_SDK_VIDEO_FORMAT_VUY2)
    {
        size_t yuv_size = imageFormatSize(IMAGE_YUYV, width, height);
        if (!check_image_size(this->img_size, yuv_size))
        {
            printf("Image Size NOT Match! [Img_Size:%ld] [YUV_Size:%ld]\n", this->img_size, yuv_size);
            return 1;
        }
        if (!this->mBufferYUV.Alloc(2, yuv_size, RingBuffer::ZeroCopy))
        {
            printf("1****CUDA YUV2RGB -- failed to allocate buffers (%zu bytes each)\n", yuv_size);
            // enc_success = false;
            return 1;
        }
        void *next_yuv = this->mBufferYUV.Next(RingBuffer::Write);
        cuda_memcpy_h2d(next_yuv, this->nextYUV, yuv_size);
    }
    else if (this->format == ALG_SDK_VIDEO_FORMAT_RAW10)
    {
        // TODO
    }
    else if (this->format == ALG_SDK_VIDEO_FORMAT_RAW12)
    {
        // TODO
    }
    else
    {
        printf("CUDA CAPTURE -- unsupported input video format (%d)\n", this->format);
        printf("                        supported formats are:\n");
        printf("                            * yuy2\n");
        printf("                            * yvyu\n");
        printf("                            * uyvy\n");
        printf("                            * vyuy\n");
        printf("                            * raw10\n");
        return 1;
    }

    frame_rate_monitor(this->frame_index);
    return 0;
}

int AlgCamera::cuda_yuv_2_rgb_converter(void *src, void *dst, int width, int height, int video_format)
{
    if (!src || width == 0 || height == 0)
    {
        printf("CUDA YUV2RGB failed : src is empty!\n");
        return 1;
    }

    printf("Input Video Format = %d\n", video_format);
    imageFormat cuda_image_format;
    switch (video_format)
    {
    case ALG_SDK_VIDEO_FORMAT_YUY2:
        cuda_image_format = IMAGE_YUYV;
        break;
    case ALG_SDK_VIDEO_FORMAT_YVYU:
        cuda_image_format = IMAGE_YVYU;
        break;
    case ALG_SDK_VIDEO_FORMAT_UYVY:
        cuda_image_format = IMAGE_UYVY;
        break;
    case ALG_SDK_VIDEO_FORMAT_VYUY:
        cuda_image_format = IMAGE_VYUY;
        break; /* TODO */
    case ALG_SDK_VIDEO_FORMAT_Y2UV:
        cuda_image_format = IMAGE_Y2UV;
        break;
    case ALG_SDK_VIDEO_FORMAT_Y2VU:
        cuda_image_format = IMAGE_Y2VU;
        break;
    case ALG_SDK_VIDEO_FORMAT_UVY2:
        cuda_image_format = IMAGE_UVY2;
        break;
    case ALG_SDK_VIDEO_FORMAT_VUY2:
        cuda_image_format = IMAGE_VUY2;
        break; /* TODO */
    default:
    {
        printf("CUDA YUV2RGB -- unsupported input video format (%d)\n", video_format);
        printf("                        supported formats are:\n");
        printf("                            * yuy2\n");
        printf("                            * yvyu\n");
        printf("                            * uyvy\n");
        printf("                            * vyuy\n");
        return 1;
    }
    }

    const size_t rgb8Size = imageFormatSize(IMAGE_RGBA8, width, height);
    if (!this->mBufferRGB.Alloc(5, rgb8Size, RingBuffer::ZeroCopy))
    {
        printf("1****CUDA YUV2RGB -- failed to allocate buffers (%zu bytes each)\n", rgb8Size);
        return 1;
    }

    void *yuv_img = this->mBufferYUV.Peek(RingBuffer::Read);
    void *next_rgb = this->mBufferRGB.Next(RingBuffer::Write);
    if (!cuda_cvtColor_RGBA(yuv_img, cuda_image_format, next_rgb, width, height))
    {
        return 1;
    }

    dst = next_rgb;
    return 0;
}

int AlgCamera::img_converter()
{
    switch (this->format)
    {
    case ALG_SDK_VIDEO_FORMAT_YUY2:
    case ALG_SDK_VIDEO_FORMAT_YVYU:
    case ALG_SDK_VIDEO_FORMAT_UYVY:
    case ALG_SDK_VIDEO_FORMAT_VYUY:
    case ALG_SDK_VIDEO_FORMAT_Y2UV:
    case ALG_SDK_VIDEO_FORMAT_Y2VU:
    case ALG_SDK_VIDEO_FORMAT_UVY2:
    case ALG_SDK_VIDEO_FORMAT_VUY2:
        cuda_yuv_2_rgb_converter(this->nextYUV, this->nextRGB, this->width, this->height, this->format);
        break;
    default:
        break;
    }
    return 0;
}

void AlgCamera::save_image_raw(const char *filename, void *image_ptr, size_t image_size)
{
    std::ofstream storage_file(filename, std::ios::out | std::ios::binary);
    storage_file.write((char *)image_ptr, image_size);
    storage_file.close();
}

void AlgCamera::save_image_bmp(const char *filename, void *image_ptr)
{
    BmpFileHeader FileHr;
    std::ofstream storage_file(filename, std::ios::out | std::ios::binary);
    unsigned short fileType = 0x4D42;
    storage_file.write((char *)&fileType, sizeof(unsigned short));

    FileHr.bfSize = img_size * 2 + 54;
    FileHr.bfReserved1 = 0;
    FileHr.bfReserved2 = 0;
    FileHr.bfOffBits = 54;
    storage_file.write((char *)&FileHr, sizeof(BmpFileHeader));

    BmpInfoHeader InfoHr;
    InfoHr.biSize = 40;
    InfoHr.biWidth = this->width;

    /* biHeight = -height : Image Not Flip
     * biHeight = height : Image Flip
     */
    InfoHr.biHeight = -this->height;
    InfoHr.biPlane = 1;
    InfoHr.biBitCount = BMP_IMAGE_RGBA / 2 * 2 > 0 ? BMP_IMAGE_RGBA / 2 * 2 : 1;
    InfoHr.biCompression = 0;

    /* Image Format : RGBA : width*4
     * Image Format : RGB : width*3
     */
    unsigned short PerLine = this->width * 4;
    short offset = PerLine % 4;
    if (offset != 0)
        PerLine += 4 - offset;

    InfoHr.biSizeImage = PerLine * this->height;
    InfoHr.biXPelsPerMeter = 0;
    InfoHr.biYPelsPerMeter = 0;
    InfoHr.biClrUsed = 0;
    InfoHr.biClrImport = 0;
    storage_file.write((char *)&InfoHr, sizeof(BmpInfoHeader));
    storage_file.write((char *)image_ptr, InfoHr.biSizeImage);
    storage_file.close();
}

int AlgCamera::render_image()
{
    // // update display
    if (this->display != NULL)
    {
        void *next_image = this->next_image();
        this->display->RenderOnce(next_image, this->width, this->height, IMAGE_RGBA8, 0, 0, 0);

        /* Save Image */
        if (this->display->GetKeyPressFlag())
        {
            this->display->SetKeyPressFlag();
            char key_str[32];
            this->display->GetKeyPressStr(key_str);
            printf("key=%d\n", key_str[0]);

            if (key_str[0] == 32)
            {
                /* key=SPACE*/
                char filename_bmp[128] = {};
                sprintf(filename_bmp, "data/image_%02d_%08d.bmp", ch_id, frame_index);
                char filename_raw[128] = {};
                sprintf(filename_raw, "data/image_%02d_%08d.raw", ch_id, frame_index);
                save_image_bmp(filename_bmp, next_image);
                save_image_raw(filename_raw, this->nextYUV, this->img_size);
            }
            else if (key_str[0] == 115 && this->b_saving_image == false)
            {
                /* key=s */
                this->b_saving_image = true;
            }
            else if (key_str[0] == 115 && this->b_saving_image == true)
            {
                this->b_saving_image = false;
            }
        }

        if (this->b_saving_image == true)
        {
            char filename_bmp[128] = {};
            sprintf(filename_bmp, "data/image_%02d_%08d.bmp", ch_id, frame_index);
            char filename_raw[128] = {};
            sprintf(filename_raw, "data/image_%02d_%08d.raw", ch_id, frame_index);
            save_image_bmp(filename_bmp, next_image);
            // save_image_raw(filename_raw, this->nextYUV, this->img_size);
        }
        // // update status bar
        char str[256];
        sprintf(str, "Channel[%02d] (%ux%u) | %.02f FPS | Timestamp: %ld", this->ch_id, this->width, this->height, this->frame_rate, this->timestamp);
        display->SetTitle(str);
        display->RefreshWindow();
    }

    return 0;
}

int AlgCamera::is_closed()
{
    return display->IsClosed();
}

bool AlgCamera::is_init()
{
    return this->b_init;
}

void *AlgCamera::next_image()
{
    return this->mBufferRGB.Peek(RingBuffer::Read);
}

bool AlgCamera::check_image_size(size_t s1, size_t s2)
{
    if (s1 == s2)
        return true;
    else
        return false;
}

void AlgCamera::frame_rate_monitor(const int frame_index)
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
        // printf("Frame Monitor : Frame drop ! [Current Frame %d] [Last Frame %d]\n", frame_index, g_f_last[ch_id]);
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
