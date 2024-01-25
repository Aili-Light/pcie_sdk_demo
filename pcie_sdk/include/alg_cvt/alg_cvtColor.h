#ifndef _ALG_CVT_H_
#define _ALG_CVT_H_

#ifdef __cplusplus
extern "C" {
#endif
#if defined(MINGW32) && defined(BUILD_EXPORT)
    #ifdef ALG_SDK_EXPORT
        #define ALG_SDK_API __declspec(dllexport)
    #else
        #define ALG_SDK_API __declspec(dllimport)
    #endif // ALG_SDK_EXPORT
#else
    #define ALG_SDK_API extern
#endif // MINGW32

namespace alg_cv
{
    enum bayer_pattern_e
    {
        ALG_CV_BAYER_PATTERN_UNKNOWN = 0, // Unkonwn pattern
        ALG_CV_BAYER_PATTERN_RGGB = 1,   // R-Gb-Gr-B Pattern
        ALG_CV_BAYER_PATTERN_BGGR = 2,   // B-Gb-Gr-R Pattern 
        ALG_CV_BAYER_PATTERN_GBRG = 3,   // Gb-B-R-Gr Pattern
        ALG_CV_BAYER_PATTERN_GRBG = 4,   // Gr-R-B-Gb Pattern
        ALG_CV_BAYER_PATTERN_RCCC = 5,   // R-C-C-C Pattern
    };

    enum color_space_e
    {
        ALG_CV_YUV2RGBCVT_ERR = 0xFF,
        ALG_CV_YUV2RGBCVT_DEFAULT = 0x0,
        ALG_CV_YUV2RGBCVT_UVY2 = 0x18,
        ALG_CV_YUV2RGBCVT_VUY2 = 0x19,
        ALG_CV_YUV2RGBCVT_Y2UV = 0x1A,
        ALG_CV_YUV2RGBCVT_Y2VU = 0x1B,
        ALG_CV_YUV2RGBCVT_UYVY = 0x1C,
        ALG_CV_YUV2RGBCVT_VYUY = 0x1D,
        ALG_CV_YUV2RGBCVT_YUYV = 0x1E,
        ALG_CV_YUV2RGBCVT_YVYU = 0x1F,
        ALG_CV_Bayer2RGB_RAW10 = 0x2B,
        ALG_CV_Bayer2RGB_RAW12 = 0x2C,
        ALG_CV_RGB2YUV_YUYV = 0x30,
        ALG_CV_RGB2YUV_YVYU = 0x31,
        ALG_CV_RGB2YUV_UYVY = 0x32,
        ALG_CV_RGB2YUV_VYUY = 0x33,
        ALG_CV_BayerBG2RGB = 0x40,
        ALG_CV_BayerGB2RGB = 0x41,
        ALG_CV_BayerRG2RGB = 0x42,
        ALG_CV_BayerGR2RGB = 0x43,
        ALG_CV_RGB2BayerBG = 0x50,
        ALG_CV_RGB2BayerGB = 0x51,
        ALG_CV_RGB2BayerRG = 0x52,
        ALG_CV_RGB2BayerGR = 0x53,
        ALG_CV_RGB2BayerRCCC = 0x54,
        ALG_CV_YUV2I420_YUYV = 0x60,
        ALG_CV_YUV2I420_YVYU = 0x61,
        ALG_CV_RGB2YCbCr_RGB = 0x70,
        ALG_CV_RGB2YCbCr_BGR = 0x71,
        ALG_CV_YUYV2UYVY = 0x80,
        ALG_CV_YUYV2VYUY = 0x81,
        ALG_CV_YUYV2YVYU = 0x82,
        ALG_CV_YUYV2UVY2 = 0x83,
        ALG_CV_YUYV2VUY2 = 0x84,
        ALG_CV_YUYV2Y2UV = 0x85,
        ALG_CV_YUYV2Y2VU = 0x86,
        ALG_CV_I4202YUV_YUYV = 0x90,
        ALG_CV_I4202YUV_YVYU = 0x91,
        ALG_CV_I4202YUV_UYVY = 0x92,
        ALG_CV_I4202YUV_VYUY = 0x93,
        ALG_CV_NV12ToYUV_YUYV = 0x94,
        ALG_CV_NV12ToYUV_YVYU = 0x95,
        ALG_CV_NV12ToYUV_UYVY = 0x96,
        ALG_CV_NV12ToYUV_VYUY = 0x97,
        ALG_CV_NV21ToYUV_YUYV = 0x98,
        ALG_CV_NV21ToYUV_YVYU = 0x99,
        ALG_CV_NV21ToYUV_UYVY = 0x9a,
        ALG_CV_NV21ToYUV_VYUY = 0x9b,
    };

    /* return color space type enum
    * @args `data_type` : input color space type
    */
    ALG_SDK_API color_space_e image_format(int data_type);

    /* Color space converter
    * @args `src` : pointer to the source image
    * @args `dst` : pointer to the result image
    * @args `w`   : width of the source image
    * @args `h`   : height of the source image
    * @args `color_space` : color space convert type (see def enum color_space_e)
    */
    ALG_SDK_API void alg_sdk_cvtColor(void *src, void *dst, int w, int h, color_space_e color_space);    
}

#ifdef __cplusplus
}
#endif
#endif
