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
#ifndef __CUDA_IMPL_H__
#define __CUDA_IMPL_H__
#include "stdint.h"
#include "jetson-utils/imageFormat.h"

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

/* Memory Copy From CPU space (src) to GPU space (dst) */
ALG_SDK_API void cuda_memcpy_h2d(void* dst, void* src, size_t size);

/* Color Convert from YUV to RGBA by CUDA 
* * img_src must point to image in GPU memory
*/
ALG_SDK_API bool cuda_cvtColor_RGBA(void* img_src, imageFormat src_format, void* img_rgba, int width, int height);

/* Allocate CUDA Memory */
ALG_SDK_API bool cuda_alloc_map(void** ptr, size_t size );

/* Color Convert from YUV to I420 by CUDA 
* img_src must point to image in GPU memory
*/
ALG_SDK_API bool cuda_cvtColor_I420(void* img_src, imageFormat src_format, void* img_rgba, int width, int height);

/* CUDA Device Sync */
ALG_SDK_API void cuda_sync_device();

/* Free CUDA Host 
* ptr must point to GPU memory
*/
ALG_SDK_API void cuda_free_host(void *ptr);

/* CUDA Malloc */
ALG_SDK_API bool cuda_malloc(void **p, size_t s);

/* CUDA Malloc */
ALG_SDK_API void cuda_free(void *ptr);

#ifdef __cplusplus
}
#endif
#endif
