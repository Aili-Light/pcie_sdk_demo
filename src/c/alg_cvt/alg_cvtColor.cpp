#include "alg_cvtColor.h"
#include "stdint.h"

#define OMP_NUM_THREADS 8
namespace alg_cv
{

#define LIMIT_VAL_HIGH(val, max) (val) > (max) ? (max) : (val)
#define LIMIT_VAL_LOW(val, min) (val) < (min) ? (min) : (val)

#define ALG_CV_YUV2RGBCVT_SHIFT 20
#define ALG_CV_YUV2RGBCVT_CY 1220542  // 1.164*1024*1024
#define ALG_CV_YUV2RGBCVT_CVR 1673527 // 1.596*1024*1024
#define ALG_CV_YUV2RGBCVT_CVG -852492 // -0.813*1024*1024
#define ALG_CV_YUV2RGBCVT_CUG -409993 // -0.391*1024*1024
#define ALG_CV_YUV2RGBCVT_CUB 2116026 // 2.018*1024*1024

#define ALG_CV_RGB2YUVCVT_CRY 313524       // 0.299*1024*1024
#define ALG_CV_RGB2YUVCVT_CGY 615514       // 0.587*1024*1024
#define ALG_CV_RGB2YUVCVT_CBY 119537       // 0.114*1024*1024
#define ALG_CV_RGB2YUVCVT_CRU -177209      // -0.169*1024*1024
#define ALG_CV_RGB2YUVCVT_CGU -347078      // -0.331*1024*1024
#define ALG_CV_RGB2YUVCVT_CBU 524288       // 0.5*1024*1024
#define ALG_CV_RGB2YUVCVT_CRV 524288       // 0.5*1024*1024
#define ALG_CV_RGB2YUVCVT_CGV -439353      // -0.419*1024*1024
#define ALG_CV_RGB2YUVCVT_CBV -84934       // -0.081*1024*1024
#define ALG_CV_RGB2YUVCVT_OFFSET 134217728 // 128*1024*1024

    color_space_e image_format(int data_type)
    {
        switch (data_type)
        {
        case 0:
            return ALG_CV_YUV2RGBCVT_DEFAULT;
        case 0x18:
            return ALG_CV_YUV2RGBCVT_UVY2;
        case 0x19:
            return ALG_CV_YUV2RGBCVT_VUY2;
        case 0x1A:
            return ALG_CV_YUV2RGBCVT_Y2UV;
        case 0x1B:
            return ALG_CV_YUV2RGBCVT_Y2VU;
        case 0x1C:
            return ALG_CV_YUV2RGBCVT_UYVY;
        case 0x1D:
            return ALG_CV_YUV2RGBCVT_VYUY;
        case 0x1E:
            return ALG_CV_YUV2RGBCVT_YUYV;
        case 0x1F:
            return ALG_CV_YUV2RGBCVT_YVYU;
        case 0x2B:
            return ALG_CV_Bayer2RGB_RAW10;
        case 0x2C:
            return ALG_CV_Bayer2RGB_RAW12;
        default:
            return ALG_CV_YUV2RGBCVT_ERR;
        }
    }

    void rgb2bayer(unsigned char *src, unsigned char *out, int w, int h, color_space_e color_space)
    {
        if (!src || !out)
        {
            return;
        }
        int bayer_step = w;
        int rgb_step = w * 3;
        int dcn = 3;
        int dcn2 = dcn * 2;
        int hh = h / 2;
        const unsigned char *rgb0 = src;
        unsigned char *bayer0 = out;
        int t0, t1, t2, t3;
        int pattern_type = 0;

        switch (color_space)
        {
        case ALG_CV_RGB2BayerRG:
        case ALG_CV_RGB2BayerBG:
        case ALG_CV_RGB2BayerGB:
        case ALG_CV_RGB2BayerGR:
            pattern_type = 0;
            break;
        case ALG_CV_RGB2BayerRCCC:
            pattern_type = 1;
            break;
        default:
            break;
        }
        if (color_space == ALG_CV_RGB2BayerRG)
        {
            // R-G-R-G
            // G-B-G-B
            t0 = 0;
            t1 = 1;
            t2 = 1;
            t3 = 2;
        }
        else if (color_space == ALG_CV_RGB2BayerBG)
        {
            // B-G-B-G
            // G-R-G-R
            t0 = 2;
            t1 = 1;
            t2 = 1;
            t3 = 0;
        }
        else if (color_space == ALG_CV_RGB2BayerGB)
        {
            // G-B-G-B
            // R-G-R-G
            t0 = 1;
            t1 = 2;
            t2 = 0;
            t3 = 1;
        }
        else if (color_space == ALG_CV_RGB2BayerGR)
        {
            // G-R-G-R
            // B-G-B-G
            t0 = 1;
            t1 = 0;
            t2 = 2;
            t3 = 1;
        }
        else if (color_space == ALG_CV_RGB2BayerRCCC)
        {
            // R-C-C-C
            t0 = 0;
            t1 = 1;
            t2 = 2;
            t3 = 0;
        }
        else
        {
            return;
        }

        if (pattern_type == 0) // R-G-B Bayer pattern
        {
            for (int i = 0; i < hh; rgb0 += rgb_step * 2, bayer0 += bayer_step * 2, ++i)
            {
                const unsigned char *rgb = rgb0;
                const unsigned char *rgb_end = rgb + rgb_step;
                unsigned char *dst = bayer0;
                for (; rgb <= rgb_end - 3; rgb += dcn2, dst += 2)
                {
                    dst[0] = rgb[t0];
                    dst[1] = rgb[t1];
                    dst[bayer_step] = rgb[rgb_step + t2];
                    dst[bayer_step + 1] = rgb[rgb_step + t3];
                }
            }
        }
        else if (pattern_type == 1) // R-C-C-C Bayer pattern
        {
            for (int i = 0; i < hh; rgb0 += rgb_step * 2, bayer0 += bayer_step * 2, ++i)
            {
                const unsigned char *rgb = rgb0;
                const unsigned char *rgb_end = rgb + rgb_step;
                unsigned char *dst = bayer0;
                for (; rgb <= rgb_end - 3; rgb += dcn2, dst += 2)
                {
                    unsigned char c = 0.299 * rgb[t0] + 0.587 * rgb[t1] + 0.114 * rgb[t2];
                    dst[0] = rgb[t0];
                    dst[1] = c;
                    dst[bayer_step] = c;
                    dst[bayer_step + 1] = c;
                }
            }
        }
    }

    void bayer2rgb(unsigned char *src, unsigned char *out, int w, int h, color_space_e color_space)
    {
        if (!src || !out)
        {
            return;
        }
        // size of rgb
        uint32_t size = w * h * 2;
        int bayer_step = w;
        int dst_step = w * 3;
        int dcn = 3;
        int dcn2 = dcn << 1;
        int start_with_green = 0;
        int blue = 1;
        const unsigned char *bayer0 = src;
        unsigned char *dst0 = out;
        if (color_space == ALG_CV_BayerBG2RGB) // Gr-R-B-Gb Pattern
        {
            start_with_green = 0;
            blue = 1;
        }
        else if (color_space == ALG_CV_BayerRG2RGB) // Gb-B-R-Gr Pattern
        {
            start_with_green = 0;
            blue = -1;
        }
        else if (color_space == ALG_CV_BayerGB2RGB) // R-Gb-Gr-B Pattern
        {
            start_with_green = 1;
            blue = 1;
        }
        else if (color_space == ALG_CV_BayerGR2RGB) // B-Gb-Gr-R Pattern
        {
            start_with_green = 1;
            blue = -1;
        }
        else
        {
            return;
        }
        for (int i = 0; i < h; bayer0 += bayer_step, dst0 += dst_step, ++i)
        {
            int t0, t1;
            const unsigned char *bayer = bayer0;
            const unsigned char *bayer_end = bayer + w;
            unsigned char *dst = dst0;
            if (start_with_green)
            {
                t0 = (bayer[1] + bayer[bayer_step * 2 + 1] + 1) >> 1;
                t1 = (bayer[bayer_step] + bayer[bayer_step + 2] + 1) >> 1;
                dst[-blue] = (unsigned char)t0;
                dst[0] = bayer[bayer_step + 1];
                dst[blue] = (unsigned char)t1;
                bayer++;
                dst += dcn;
            }
            if (dcn == 3) // Bayer to BGR
            {
                if (blue > 0)
                {
                    for (; bayer <= bayer_end - 2; bayer += 2, dst += dcn2)
                    {
                        t0 = (bayer[0] + bayer[2] + bayer[bayer_step * 2] +
                              bayer[bayer_step * 2 + 2] + 2) >>
                             2;
                        t1 = (bayer[1] + bayer[bayer_step] +
                              bayer[bayer_step + 2] + bayer[bayer_step * 2 + 1] + 2) >>
                             2;
                        dst[-1] = (unsigned char)t0;
                        dst[0] = (unsigned char)t1;
                        dst[1] = bayer[bayer_step + 1];
                        t0 = (bayer[2] + bayer[bayer_step * 2 + 2] + 1) >> 1;
                        t1 = (bayer[bayer_step + 1] + bayer[bayer_step + 3] + 1) >> 1;
                        dst[2] = (unsigned char)t0;
                        dst[3] = bayer[bayer_step + 2];
                        dst[4] = (unsigned char)t1;
                    }
                }
                else
                {
                    for (; bayer <= bayer_end - 2; bayer += 2, dst += dcn2)
                    {
                        t0 = (bayer[0] + bayer[2] + bayer[bayer_step * 2] +
                              bayer[bayer_step * 2 + 2] + 2) >>
                             2;
                        t1 = (bayer[1] + bayer[bayer_step] +
                              bayer[bayer_step + 2] + bayer[bayer_step * 2 + 1] + 2) >>
                             2;
                        dst[1] = (unsigned char)t0;
                        dst[0] = (unsigned char)t1;
                        dst[-1] = bayer[bayer_step + 1];
                        t0 = (bayer[2] + bayer[bayer_step * 2 + 2] + 1) >> 1;
                        t1 = (bayer[bayer_step + 1] + bayer[bayer_step + 3] + 1) >> 1;
                        dst[4] = (unsigned char)t0;
                        dst[3] = bayer[bayer_step + 2];
                        dst[2] = (unsigned char)t1;
                    }
                }
            }
            // if skip one pixel at the end of row
            if (bayer < bayer_end)
            {
                t0 = (bayer[0] + bayer[2] + bayer[bayer_step * 2] +
                      bayer[bayer_step * 2 + 2] + 2) >>
                     2;
                t1 = (bayer[1] + bayer[bayer_step] +
                      bayer[bayer_step + 2] + bayer[bayer_step * 2 + 1] + 2) >>
                     2;
                dst[-blue] = (unsigned char)t0;
                dst[0] = (unsigned char)t1;
                dst[blue] = bayer[bayer_step + 1];
                bayer++;
                dst += dcn;
            }
            // fill the last and the first pixels of row accordingly
            if (dcn == 3)
            {
                dst0[-4] = dst0[-1];
                dst0[-3] = dst0[0];
                dst0[-2] = dst0[1];
                dst0[w * dcn - 1] = dst0[w * dcn - 4];
                dst0[w * dcn] = dst0[w * dcn - 3];
                dst0[w * dcn + 1] = dst0[w * dcn - 2];
            }
            blue = -blue;
            start_with_green = !start_with_green;
        }
    }

    void rgb2yuv(unsigned char *src, unsigned char *out, int w, int h, color_space_e color_space)
    {
        if (!src || !out)
        {
            return;
        }
        if (color_space == ALG_CV_RGB2YUV_YUYV || color_space == ALG_CV_RGB2YUV_YVYU)
        {
            for (int index = 0; index < w * h / 2; ++index)
            {
                unsigned char r1 = src[index * 6 + 0];
                unsigned char g1 = src[index * 6 + 1];
                unsigned char b1 = src[index * 6 + 2];
                unsigned char r2 = src[index * 6 + 3];
                unsigned char g2 = src[index * 6 + 4];
                unsigned char b2 = src[index * 6 + 5];
                float y1_f = 0.299 * r1 + 0.587 * g1 + 0.114 * b1;
                float u_f = -0.169 * r1 - 0.331 * g1 + 0.5 * b1 + 128;
                float v_f = 0.5 * r1 - 0.419 * g1 - 0.081 * b1 + 128;
                float y2_f = 0.299 * r2 + 0.587 * g2 + 0.114 * b2;
                y1_f = LIMIT_VAL_HIGH(y1_f, 255);
                y2_f = LIMIT_VAL_HIGH(y2_f, 255);
                u_f = LIMIT_VAL_HIGH(u_f, 255);
                v_f = LIMIT_VAL_HIGH(v_f, 255);
                y1_f = LIMIT_VAL_LOW(y1_f, 0);
                y2_f = LIMIT_VAL_LOW(y2_f, 0);
                u_f = LIMIT_VAL_LOW(u_f, 0);
                v_f = LIMIT_VAL_LOW(v_f, 0);
                if (color_space == ALG_CV_RGB2YUV_YUYV)
                {
                    out[index * 4 + 0] = y1_f;
                    out[index * 4 + 1] = u_f;
                    out[index * 4 + 2] = y2_f;
                    out[index * 4 + 3] = v_f;
                }
                else if (color_space == ALG_CV_RGB2YUV_YVYU)
                {
                    out[index * 4 + 0] = y1_f;
                    out[index * 4 + 1] = v_f;
                    out[index * 4 + 2] = y2_f;
                    out[index * 4 + 3] = u_f;
                }
            }
        }
    }

    void rgb888_yuv422_converter(unsigned char *img_src, unsigned char *img_dst, uint32_t index, uint32_t pos, int *yuv_idx)
    {
        unsigned char r1 = img_src[index * 6 + 0];
        unsigned char g1 = img_src[index * 6 + 1];
        unsigned char b1 = img_src[index * 6 + 2];
        unsigned char r2 = img_src[index * 6 + 3];
        unsigned char g2 = img_src[index * 6 + 4];
        unsigned char b2 = img_src[index * 6 + 5];
        uint32_t y1_s = ALG_CV_RGB2YUVCVT_CRY * r1 + ALG_CV_RGB2YUVCVT_CGY * g1 + ALG_CV_RGB2YUVCVT_CBY * b1;
        uint32_t u_s = ALG_CV_RGB2YUVCVT_CRU * r1 + ALG_CV_RGB2YUVCVT_CGU * g1 + ALG_CV_RGB2YUVCVT_CBU * b1 + ALG_CV_RGB2YUVCVT_OFFSET;
        uint32_t v_s = ALG_CV_RGB2YUVCVT_CRV * r1 + ALG_CV_RGB2YUVCVT_CGV * g1 + ALG_CV_RGB2YUVCVT_CBV * b1 + ALG_CV_RGB2YUVCVT_OFFSET;
        uint32_t y2_s = ALG_CV_RGB2YUVCVT_CRY * r2 + ALG_CV_RGB2YUVCVT_CGY * g2 + ALG_CV_RGB2YUVCVT_CBY * b2;
        int y1 = int(y1_s >> ALG_CV_YUV2RGBCVT_SHIFT);
        int y2 = int(y2_s >> ALG_CV_YUV2RGBCVT_SHIFT);
        int u = int(u_s >> ALG_CV_YUV2RGBCVT_SHIFT);
        int v = int(v_s >> ALG_CV_YUV2RGBCVT_SHIFT);
        int y10 = LIMIT_VAL_LOW(y1, 0);
        int y20 = LIMIT_VAL_LOW(y2, 0);
        int u0 = LIMIT_VAL_LOW(u, 0);
        int v0 = LIMIT_VAL_LOW(v, 0);
        img_dst[pos + yuv_idx[0]] = LIMIT_VAL_HIGH(y10, 255);
        img_dst[pos + yuv_idx[1]] = LIMIT_VAL_HIGH(y20, 255);
        img_dst[pos + yuv_idx[2]] = LIMIT_VAL_HIGH(u0, 255);
        img_dst[pos + yuv_idx[3]] = LIMIT_VAL_HIGH(v0, 255);
    }

    void yuv422_rgb888_converter(unsigned char *img_src, unsigned char *img_dst, uint32_t index, uint32_t pos, int *yuv_idx)
    {
        int y1 = int(img_src[4 * index + yuv_idx[0]] - 16) * ALG_CV_YUV2RGBCVT_CY;
        int y2 = int(img_src[4 * index + yuv_idx[1]] - 16) * ALG_CV_YUV2RGBCVT_CY;
        int u = int(img_src[4 * index + yuv_idx[2]]) - 128;
        int v = int(img_src[4 * index + yuv_idx[3]]) - 128;

        int y10 = LIMIT_VAL_LOW(y1, 0);
        int y20 = LIMIT_VAL_LOW(y2, 0);
        int ruv = LIMIT_VAL_LOW(ALG_CV_YUV2RGBCVT_CVR * v, 0);
        int guv = LIMIT_VAL_LOW(ALG_CV_YUV2RGBCVT_CVG * v + ALG_CV_YUV2RGBCVT_CUG * u, 0);
        int buv = LIMIT_VAL_LOW(ALG_CV_YUV2RGBCVT_CUB * u, 0);

        img_dst[pos + 0] = LIMIT_VAL_HIGH((y10 + ruv) >> ALG_CV_YUV2RGBCVT_SHIFT, 255);
        img_dst[pos + 1] = LIMIT_VAL_HIGH((y10 + guv) >> ALG_CV_YUV2RGBCVT_SHIFT, 255);
        img_dst[pos + 2] = LIMIT_VAL_HIGH((y10 + buv) >> ALG_CV_YUV2RGBCVT_SHIFT, 255);
        img_dst[pos + 3] = LIMIT_VAL_HIGH((y20 + ruv) >> ALG_CV_YUV2RGBCVT_SHIFT, 255);
        img_dst[pos + 4] = LIMIT_VAL_HIGH((y20 + guv) >> ALG_CV_YUV2RGBCVT_SHIFT, 255);
        img_dst[pos + 5] = LIMIT_VAL_HIGH((y20 + buv) >> ALG_CV_YUV2RGBCVT_SHIFT, 255);
    }

    void set_yuv_index(int *yuv_idx, color_space_e color_space)
    {
        if (!yuv_idx)
        {
            return;
        }

        if (color_space == ALG_CV_YUV2RGBCVT_DEFAULT)
        {
            yuv_idx[0] = 0;
            yuv_idx[1] = 2;
            yuv_idx[2] = 1;
            yuv_idx[3] = 3;
        }
        else if (color_space == ALG_CV_YUV2RGBCVT_UYVY)
        {
            yuv_idx[0] = 1;
            yuv_idx[1] = 3;
            yuv_idx[2] = 0;
            yuv_idx[3] = 2;
        }
        else if (color_space == ALG_CV_YUV2RGBCVT_VYUY)
        {
            yuv_idx[0] = 1;
            yuv_idx[1] = 3;
            yuv_idx[2] = 2;
            yuv_idx[3] = 0;
        }
        else if (color_space == ALG_CV_YUV2RGBCVT_YUYV)
        {
            yuv_idx[0] = 0;
            yuv_idx[1] = 2;
            yuv_idx[2] = 1;
            yuv_idx[3] = 3;
        }
        else if (color_space == ALG_CV_YUV2RGBCVT_YVYU)
        {
            yuv_idx[0] = 0;
            yuv_idx[1] = 2;
            yuv_idx[2] = 3;
            yuv_idx[3] = 1;
        }
        else if (color_space == ALG_CV_YUV2RGBCVT_UVY2)
        {
            yuv_idx[0] = 3;
            yuv_idx[1] = 1;
            yuv_idx[2] = 0;
            yuv_idx[3] = 2;
        }
        else if (color_space == ALG_CV_YUV2RGBCVT_VUY2)
        {
            yuv_idx[0] = 3;
            yuv_idx[1] = 1;
            yuv_idx[2] = 2;
            yuv_idx[3] = 0;
        }
        else if (color_space == ALG_CV_YUV2RGBCVT_Y2UV)
        {
            yuv_idx[0] = 2;
            yuv_idx[1] = 0;
            yuv_idx[2] = 1;
            yuv_idx[3] = 3;
        }
        else if (color_space == ALG_CV_YUV2RGBCVT_Y2VU)
        {
            yuv_idx[0] = 2;
            yuv_idx[1] = 0;
            yuv_idx[2] = 3;
            yuv_idx[3] = 1;
        }
    }

    void cvt_yuv422_rgb888_loop(unsigned char *img_src, unsigned char *img_dst, int section, uint32_t data_size_div, int *yuv_idx)
    {
        uint32_t pos = 6 * data_size_div * section;
        for (uint32_t index = section * data_size_div; index < (section + 1) * data_size_div; index++, pos += 6)
        {
            yuv422_rgb888_converter(img_src, img_dst, index, pos, yuv_idx);
        }
    }

    void cvt_rgb888_yuv422_loop(unsigned char *img_src, unsigned char *img_dst, int section, uint32_t data_size_div, int *yuv_idx)
    {
        uint32_t pos = 4 * data_size_div * section;
        for (uint32_t index = section * data_size_div; index < (section + 1) * data_size_div; index++, pos += 4)
        {
            rgb888_yuv422_converter(img_src, img_dst, index, pos, yuv_idx);
        }
    }

    void cvt_yuv422_rgb888_impl(unsigned char *img_src, unsigned char *img_dst, const uint32_t data_size, color_space_e color_space)
    {
        int yuv_idx[4];
        set_yuv_index(yuv_idx, color_space);
#if defined(WITH_OPENMP)
        omp_set_num_threads(OMP_NUM_THREADS);
        const uint32_t data_size_div = data_size / 2 / OMP_NUM_THREADS;

#pragma omp parallel
        {
#pragma omp sections
            {
#pragma omp section
                {
                    cvt_yuv422_rgb888_loop(img_src, img_dst, 0, data_size_div, yuv_idx);
                }
#pragma omp section
                {
                    cvt_yuv422_rgb888_loop(img_src, img_dst, 1, data_size_div, yuv_idx);
                }
#pragma omp section
                {
                    cvt_yuv422_rgb888_loop(img_src, img_dst, 2, data_size_div, yuv_idx);
                }
#pragma omp section
                {
                    cvt_yuv422_rgb888_loop(img_src, img_dst, 3, data_size_div, yuv_idx);
                }
#pragma omp section
                {
                    cvt_yuv422_rgb888_loop(img_src, img_dst, 4, data_size_div, yuv_idx);
                }
#pragma omp section
                {
                    cvt_yuv422_rgb888_loop(img_src, img_dst, 5, data_size_div, yuv_idx);
                }
#pragma omp section
                {
                    cvt_yuv422_rgb888_loop(img_src, img_dst, 6, data_size_div, yuv_idx);
                }
#pragma omp section
                {
                    cvt_yuv422_rgb888_loop(img_src, img_dst, 7, data_size_div, yuv_idx);
                }
            }
        }
#else
        const uint32_t data_size_div = data_size / 2;
        cvt_yuv422_rgb888_loop(img_src, img_dst, 0, data_size_div, yuv_idx);
#endif
    }

    void cvt_rgb888_yuv422_impl(unsigned char *img_src, unsigned char *img_dst, const uint32_t data_size, color_space_e color_space)
    {
        int yuv_idx[4];
        set_yuv_index(yuv_idx, color_space);
#if defined(WITH_OPENMP)
        omp_set_num_threads(OMP_NUM_THREADS);
        const uint32_t data_size_div = data_size / 2 / OMP_NUM_THREADS;

#pragma omp parallel
        {
#pragma omp sections
            {
#pragma omp section
                {
                    cvt_rgb888_yuv422_loop(img_src, img_dst, 0, data_size_div, yuv_idx);
                }

#pragma omp section
                {
                    cvt_rgb888_yuv422_loop(img_src, img_dst, 1, data_size_div, yuv_idx);
                }

#pragma omp section
                {
                    cvt_rgb888_yuv422_loop(img_src, img_dst, 2, data_size_div, yuv_idx);
                }

#pragma omp section
                {
                    cvt_rgb888_yuv422_loop(img_src, img_dst, 3, data_size_div, yuv_idx);
                }

#pragma omp section
                {
                    cvt_rgb888_yuv422_loop(img_src, img_dst, 4, data_size_div, yuv_idx);
                }

#pragma omp section
                {
                    cvt_rgb888_yuv422_loop(img_src, img_dst, 5, data_size_div, yuv_idx);
                }

#pragma omp section
                {
                    cvt_rgb888_yuv422_loop(img_src, img_dst, 6, data_size_div, yuv_idx);
                }

#pragma omp section
                {
                    cvt_rgb888_yuv422_loop(img_src, img_dst, 7, data_size_div, yuv_idx);
                }
            }
        }
#else
        const uint32_t data_size_div = data_size / 2;
        cvt_rgb888_yuv422_loop(img_src, img_dst, 0, data_size_div, yuv_idx);
#endif
    }

    void alg_sdk_cvtColor(void *src, void *dst, int w, int h, color_space_e color_space)
    {
        if (!src || !dst)
        {
            return;
        }
        unsigned char *img_src = (unsigned char *)src;
        unsigned char *img_dst = (unsigned char *)dst;
        const uint32_t data_size = w * h;

        switch (color_space)
        {
        case ALG_CV_YUV2RGBCVT_DEFAULT:
        case ALG_CV_YUV2RGBCVT_UVY2:
        case ALG_CV_YUV2RGBCVT_VUY2:
        case ALG_CV_YUV2RGBCVT_Y2UV:
        case ALG_CV_YUV2RGBCVT_Y2VU:
        case ALG_CV_YUV2RGBCVT_UYVY:
        case ALG_CV_YUV2RGBCVT_VYUY:
        case ALG_CV_YUV2RGBCVT_YUYV:
        case ALG_CV_YUV2RGBCVT_YVYU:
            cvt_yuv422_rgb888_impl(img_src, img_dst, data_size, color_space);
            break;
        case ALG_CV_RGB2YUV_YUYV:
        case ALG_CV_RGB2YUV_YVYU:
        case ALG_CV_RGB2YUV_UYVY:
        case ALG_CV_RGB2YUV_VYUY:
            cvt_rgb888_yuv422_impl(img_src, img_dst, data_size, color_space);
            break;
        default:
            break;
        }
    }
}
