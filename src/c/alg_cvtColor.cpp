#include "alg_cvtColor.h"
#include "stdint.h"

namespace alg_cv {

#define LIMIT_VAL_HIGH(val, max) (val) > (max) ? (max) : (val)
#define LIMIT_VAL_LOW(val, min) (val) < (min) ? (min) : (val)

#define ALG_CV_YUV2RGBCVT_SHIFT 10
#define ALG_CV_YUV2RGBCVT_CY 1192  // 1.164*1024
#define ALG_CV_YUV2RGBCVT_CVR 1634 // 1.596*1024
#define ALG_CV_YUV2RGBCVT_CVG -832 // -0.813*1024
#define ALG_CV_YUV2RGBCVT_CUG -400 // -0.391*1024
#define ALG_CV_YUV2RGBCVT_CUB 2066 // 2.018*1024

color_space_e image_format( int data_type )
{
    switch(data_type)
    {
        case 0    :  return ALG_CV_YUV2RGBCVT_DEFAULT;
        case 0x18 :  return ALG_CV_YUV2RGBCVT_UVY2;
        case 0x19 :  return ALG_CV_YUV2RGBCVT_VUY2;
        case 0x1A :  return ALG_CV_YUV2RGBCVT_Y2UV;
        case 0x1B :  return ALG_CV_YUV2RGBCVT_Y2VU;
        case 0x1C :  return ALG_CV_YUV2RGBCVT_UYVY;
        case 0x1D :  return ALG_CV_YUV2RGBCVT_VYUY;
        case 0x1E :  return ALG_CV_YUV2RGBCVT_YUYV;
        case 0x1F :  return ALG_CV_YUV2RGBCVT_YVYU;
        case 0x2B :  return ALG_CV_Bayer2RGB_RAW10;
        case 0x2C :  return ALG_CV_Bayer2RGB_RAW12;
        default   : return ALG_CV_YUV2RGBCVT_ERR;
    }
}

void alg_sdk_cvtColor(void* src, void* dst, int w, int h, color_space_e color_space)
{
    if (!src || !dst)
    {
        return;
    }
    unsigned char* img_src = (unsigned char*)src;
    unsigned char* img_dst = (unsigned char*)dst;

    const uint32_t data_size = w * h * 2;
    int y1_idx, y2_idx, u_idx, v_idx;

    if (color_space == ALG_CV_YUV2RGBCVT_DEFAULT)
    {
        y1_idx = 0;
        y2_idx = 2;
        u_idx  = 1;
        v_idx  = 3;
    }
    else if (color_space == ALG_CV_YUV2RGBCVT_UYVY)
    {
        y1_idx = 1;
        y2_idx = 3;
        u_idx  = 0;
        v_idx  = 2;
    }
    else if (color_space == ALG_CV_YUV2RGBCVT_VYUY)
    {
        y1_idx = 1;
        y2_idx = 3;
        u_idx  = 2;
        v_idx  = 0;
    }
    else if (color_space == ALG_CV_YUV2RGBCVT_YUYV)
    {
        y1_idx = 0;
        y2_idx = 2;
        u_idx  = 1;
        v_idx  = 3;
    }
    else if (color_space == ALG_CV_YUV2RGBCVT_YVYU)
    {
        y1_idx = 0;
        y2_idx = 2;
        u_idx  = 3;
        v_idx  = 1;
    }
    else if (color_space == ALG_CV_YUV2RGBCVT_UVY2)
    {
        y1_idx = 3;
        y2_idx = 1;
        u_idx  = 0;
        v_idx  = 2;
    }
    else if (color_space == ALG_CV_YUV2RGBCVT_VUY2)
    {
        y1_idx = 3;
        y2_idx = 1;
        u_idx  = 2;
        v_idx  = 0;
    }
    else if (color_space == ALG_CV_YUV2RGBCVT_Y2UV)
    {
        y1_idx = 2;
        y2_idx = 0;
        u_idx  = 1;
        v_idx  = 3;
    }
    else if (color_space == ALG_CV_YUV2RGBCVT_Y2VU)
    {
        y1_idx = 2;
        y2_idx = 0;
        u_idx  = 3;
        v_idx  = 1;
    }

    uint32_t index, pos;
    for (index = 0, pos = 0; index < data_size; index+=4, pos+=6)
    {
        int y1 = LIMIT_VAL_LOW(int(img_src[index + y1_idx] - 16) * ALG_CV_YUV2RGBCVT_CY,0);
        int y2 = LIMIT_VAL_LOW(int(img_src[index + y2_idx] - 16) * ALG_CV_YUV2RGBCVT_CY,0);
        int u =  int(img_src[index + u_idx] - 128);
        int v =  int(img_src[index + v_idx] - 128);

        int ruv = (1<<ALG_CV_YUV2RGBCVT_SHIFT) + ALG_CV_YUV2RGBCVT_CVR * v;
        int guv = (1<<ALG_CV_YUV2RGBCVT_SHIFT) + ALG_CV_YUV2RGBCVT_CVG * v + ALG_CV_YUV2RGBCVT_CUG * u;
        int buv = (1<<ALG_CV_YUV2RGBCVT_SHIFT) + ALG_CV_YUV2RGBCVT_CUB * u;

        img_dst[pos + 0] = (LIMIT_VAL_HIGH((y1 + ruv),(0xFF<<10))) >> ALG_CV_YUV2RGBCVT_SHIFT;
        img_dst[pos + 1] = (LIMIT_VAL_HIGH((y1 + guv),(0xFF<<10))) >> ALG_CV_YUV2RGBCVT_SHIFT;
        img_dst[pos + 2] = (LIMIT_VAL_HIGH((y1 + buv),(0xFF<<10))) >> ALG_CV_YUV2RGBCVT_SHIFT;

        img_dst[pos + 3] = (LIMIT_VAL_HIGH((y2 + ruv),(0xFF<<10))) >> ALG_CV_YUV2RGBCVT_SHIFT;
        img_dst[pos + 4] = (LIMIT_VAL_HIGH((y2 + guv),(0xFF<<10))) >> ALG_CV_YUV2RGBCVT_SHIFT;
        img_dst[pos + 5] = (LIMIT_VAL_HIGH((y2 + buv),(0xFF<<10))) >> ALG_CV_YUV2RGBCVT_SHIFT;
    }
}
}

