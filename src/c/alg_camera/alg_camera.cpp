#include "alg_camera.h"
#include "alg_common/log.h"
#include "jetson-utils/cuda_impl.h"
#define SAFE_DELETE(x) 		if(x != NULL) { delete x; x = NULL; }

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
    if(msg == NULL)
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
        // printf("[channel = %d], [frame = %d], [time %ld], [byte_0 = %d], [byte_end = %d]\n", this->ch_id,
        // metadata->frame_index,  metadata->timestamp, ((uint8_t*)this->nextYUV)[0], ((uint8_t*)this->nextYUV)[metadata->img_size - 1]);
    }
    else if (this->flag_src == ALG_CAMERA_FLAG_SOURCE_V4L2)
    {
        // todo : unpack v4l2 data
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
        this->format == ALG_SDK_VIDEO_FORMAT_UVY2 || this->format == ALG_SDK_VIDEO_FORMAT_VUY2 )
    {
        size_t yuv_size = imageFormatSize(IMAGE_YUYV, width, height);
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

    return 0;
}

int AlgCamera::cuda_yuv_2_rgb_converter(void *src, void *dst, int width, int height, int video_format)
{
    if (!src || width == 0 || height == 0)
    {
        printf("CUDA YUV2RGB failed : src is empty!\n");
        return 1;
    }

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
    if (!this->mBufferRGB.Alloc(2, rgb8Size, RingBuffer::ZeroCopy))
    {
        printf("1****CUDA YUV2RGB -- failed to allocate buffers (%zu bytes each)\n", rgb8Size);
        return 1;
    }

    void *yuv_img = this->mBufferYUV.Peek(RingBuffer::Read);
    void *next_rgb = this->mBufferRGB.Next(RingBuffer::Write);
    if(!cuda_cvtColor_RGBA(yuv_img, cuda_image_format, next_rgb, width, height))
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

int AlgCamera::render_image()
{
    // // update display
    if (this->display != NULL)
    {
        this->display->RenderOnce(this->next_image(), this->width, this->height, IMAGE_RGBA8, 0, 0, 0);

        // // update status bar
        char str[256];
        sprintf(str, "Channel[%02d] (%ux%u) | %.02f FPS", this->ch_id, this->width, this->height, display->GetFPS());
        display->SetTitle(str);
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