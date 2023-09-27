#ifndef _ALG_ISP_H_
#define _ALG_ISP_H_

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
    enum awb_method_e
    {
        ALG_CV_AWB_DEFAULT = 0x0,
        ALG_CV_AWB_DYM_THRESHOLD = 0x1,
    };

    /* ISP AWB Module
    * @args `src` : pointer to the source image (RGB-24bit Image)
    * @args `dst` : pointer to the result image (RGB-24bit Image)
    * @args `w`   : width of the source image
    * @args `h`   : height of the source image
    * @args `awb_method` : AWB method (default : Dynamic Threshold)
    */
    ALG_SDK_API void alg_sdk_awb(void *src, void *dst, int w, int h, awb_method_e awb_method);
}

#ifdef __cplusplus
}
#endif
#endif