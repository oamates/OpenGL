#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include "normalmap.hpp"

#include <glm/glm.hpp>

static int sample_alpha_map(unsigned char *pixels, int x, int y, int w, int h, int sw, int sh);

static inline int icerp(int a, int b, int c, int d, int x)
{
   int p = (d - c) - (a - b);
   int q = (a - b) - p;
   int r = c - a;
   return((x * (x * (x * p + (q << 7)) + (r << 14)) + (b << 21)) >> 21);
}

void scale_pixels(unsigned char* dst, int dw, int dh, unsigned char* src, int sw, int sh, int bpp)
{
    int x, y, n, ix, iy, wx, wy, v;
    int a, b, c, d;
    int dstride = dw * bpp;
    unsigned char *s;
   
    for(y = 0; y < dh; ++y)
    {
        if(dh > 1)
        {
            iy = (((sh - 1) * y) << 7) / (dh - 1);
            if(y == dh - 1) --iy;
            wy = iy & 0x7f;
            iy >>= 7;
        }
        else
            iy = wy = 0;
      
        for(x = 0; x < dw; ++x)
        {
            if(dw > 1)
            {
                ix = (((sw - 1) * x) << 7) / (dw - 1);
                if(x == dw - 1) --ix;
                wx = ix & 0x7f;
                ix >>= 7;
            }
            else
                ix = wx = 0;
         
            s = src + ((iy - 1) * sw + (ix - 1)) * bpp;
         
            for(n = 0; n < bpp; ++n)
            {
                b = icerp(s[(sw + 0) * bpp], s[(sw + 1) * bpp], s[(sw + 2) * bpp], s[(sw + 3) * bpp], wx);
                a = (iy > 0) ? icerp(s[0], s[bpp], s[2 * bpp], s[3 * bpp], wx) : b;
            
                c = icerp(s[(2 * sw + 0) * bpp], s[(2 * sw + 1) * bpp], s[(2 * sw + 2) * bpp], s[(2 * sw + 3) * bpp], wx);
                d = (iy < dh - 1) ? icerp(s[(3 * sw + 0) * bpp], s[(3 * sw + 1) * bpp], s[(3 * sw + 2) * bpp], s[(3 * sw + 3) * bpp], wx) : c;
            
                v = icerp(a, b, c, d, wy);
                if(v < 0) v = 0;
                if(v > 255) v = 255;
                dst[(y * dstride) + (x * bpp) + n] = v;
                ++s;
            }
        }
    }
}

static const float oneover255 = 1.0f / 255.0f;

static inline void NORMALIZE(float* v)
{
    float l = glm::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
   
    if(l > 1e-04f)
    {
        l = 1.0f / l;
        v[0] *= l;
        v[1] *= l;
        v[2] *= l;
    }
    else
        v[0] = v[1] = v[2] = 0.0f;
}

static void make_kernel(kernel_element_t* k, float* weights, int size)
{
    int half_size = size / 2;
    for(int y = 0; y < size; ++y)
        for(int x = 0; x < size; ++x)
        {
            int idx = x + y * size;
            k[idx] = {x - half_size, half_size - y, weights[idx]};
        }
}

static void rotate_array(float *dst, float *src, int size)
{
    for(int y = 0; y < size; ++y)
    {
        for(int x = 0; x < size; ++x)
        {
            int newy = size - x - 1;
            int newx = y;
            dst[newx + newy * size] = src[x + y * size];
        }
    }
}

int sample_alpha_map(unsigned char *pixels, int x, int y, int w, int h, int sw, int sh)
{
    int ix, iy, wx, wy, v;
    int a, b, c, d;
    unsigned char *s;
   
    if(sh > 1)
    {
        iy = (((h - 1) * y) << 7) / (sh - 1);
        if(y == sh - 1) --iy;
        wy = iy & 0x7f;
        iy >>= 7;
    }
    else
        iy = wy = 0;
   
    if(sw > 1)
    {
        ix = (((w - 1) * x) << 7) / (sw - 1);
        if(x == sw - 1) --ix;
        wx = ix & 0x7f;
        ix >>= 7;
    }
    else
        ix = wx = 0;
   
    s = pixels + ((iy - 1) * w + (ix - 1));
   
    b = icerp(s[w + 0], s[w + 1], s[w + 2], s[w + 3], wx);
    a = (iy > 0) ? icerp(s[0], s[1], s[2], s[3], wx) : b;
   
    c = icerp(s[2 * w + 0], s[2 * w + 1], s[2 * w + 2], s[2 * w + 3], wx);
    d = (iy < sh - 1) ? icerp(s[3 * w + 0], s[3 * w + 1], s[3 * w + 2], s[3 * w + 3], wx) : c;
   
    v = icerp(a, b, c, d, wy);
            
    if(v <   0) v = 0;
    if(v > 255) v = 255;
   
    return (unsigned char) v;
}

static void make_heightmap(unsigned char* image, int w, int h, int bpp, const normalmap_params_t& normalmap_params)
{
    unsigned int i, num_pixels = w * h;
    int x, y;
    float v, hmin, hmax;
   
    float* s = (float*) malloc(w * h * 3 * sizeof(float));
    float* r = (float*) calloc(w * h * 4, sizeof(float));
   
    /* scale into 0 to 1 range, make signed -1 to 1 */
    for(i = 0; i < num_pixels; ++i)
    {
        s[3 * i + 0] = (((float)image[bpp * i + 0] / 255.0f) - 0.5) * 2.0f;
        s[3 * i + 1] = (((float)image[bpp * i + 1] / 255.0f) - 0.5) * 2.0f;
        s[3 * i + 2] = (((float)image[bpp * i + 2] / 255.0f) - 0.5) * 2.0f;
    }

#define S(x, y, n) s[(y) * (w * 3) + ((x) * 3) + (n)]
#define R(x, y, n) r[(y) * (w * 4) + ((x) * 4) + (n)]
   
    /* top-left to bottom-right */
    for(x = 1; x < w; ++x)
        R(x, 0, 0) = R(x - 1, 0, 0) + S(x - 1, 0, 0);
    for(y = 1; y < h; ++y)
        R(0, y, 0) = R(0, y - 1, 0) + S(0, y - 1, 1);
    for(y = 1; y < h; ++y)
        for(x = 1; x < w; ++x)
            R(x, y, 0) = (R(x, y - 1, 0) + R(x - 1, y, 0) + S(x - 1, y, 0) + S(x, y - 1, 1)) * 0.5f;

    /* top-right to bottom-left */
    for(x = w - 2; x >= 0; --x)
        R(x, 0, 1) = R(x + 1, 0, 1) - S(x + 1, 0, 0);
    for(y = 1; y < h; ++y)
        R(0, y, 1) = R(0, y - 1, 1) + S(0, y - 1, 1);
    for(y = 1; y < h; ++y)
        for(x = w - 2; x >= 0; --x)
            R(x, y, 1) = (R(x, y - 1, 1) + R(x + 1, y, 1) - S(x + 1, y, 0) + S(x, y - 1, 1)) * 0.5f;

    /* bottom-left to top-right */
    for(x = 1; x < w; ++x)
        R(x, 0, 2) = R(x - 1, 0, 2) + S(x - 1, 0, 0);
    for(y = h - 2; y >= 0; --y)
        R(0, y, 2) = R(0, y + 1, 2) - S(0, y + 1, 1);
    for(y = h - 2; y >= 0; --y)
        for(x = 1; x < w; ++x)
            R(x, y, 2) = (R(x, y + 1, 2) + R(x - 1, y, 2) + S(x - 1, y, 0) - S(x, y + 1, 1)) * 0.5f;

    /* bottom-right to top-left */
    for(x = w - 2; x >= 0; --x)
        R(x, 0, 3) = R(x + 1, 0, 3) - S(x + 1, 0, 0);
    for(y = h - 2; y >= 0; --y)
        R(0, y, 3) = R(0, y + 1, 3) - S(0, y + 1, 1);
    for(y = h - 2; y >= 0; --y)
        for(x = w - 2; x >= 0; --x)
            R(x, y, 3) = (R(x, y + 1, 3) + R(x + 1, y, 3) - S(x + 1, y, 0) - S(x, y + 1, 1)) * 0.5f;

#undef S
#undef R

    /* accumulate, find min/max */
    hmin =  1e10f;
    hmax = -1e10f;
    for(i = 0; i < num_pixels; ++i)
    {
        r[4 * i] += r[4 * i + 1] + r[4 * i + 2] + r[4 * i + 3];
        if(r[4 * i] < hmin) hmin = r[4 * i];
        if(r[4 * i] > hmax) hmax = r[4 * i];
    }

    /* scale into 0 - 1 range */
    for(i = 0; i < num_pixels; ++i)
    {   
        v = (r[4 * i] - hmin) / (hmax - hmin);
        /* adjust contrast */
        v = (v - 0.5f) * normalmap_params.contrast + v;
        if(v < 0) v = 0;
        if(v > 1) v = 1;
        r[4 * i] = v;
    }

    /* write out results */
    for(i = 0; i < num_pixels; ++i)
    {
        v = r[4 * i] * 255.0f;
        image[bpp * i + 0] = (unsigned char) v;
        image[bpp * i + 1] = (unsigned char) v;
        image[bpp * i + 2] = (unsigned char) v;
    }

    free(s);
    free(r);
}

void normalmap(unsigned char* rgba_src, unsigned char* rgba_dst, int width, int height, const normalmap_params_t& normalmap_params)
{
    int bpp = 4;
    int x, y;
    int pw, ph, amap_w = 0, amap_h = 0;
    unsigned char *d, *s, *tmp, *amap = 0;
    float val, du, dv, n[3], weight;
    float rgb_bias[3];
    int i;

    int rowbytes = width * bpp;

    float* heights = (float*) calloc(width * height, sizeof(float));

    int kernel_size;
    kernel_element_t *kernel_du, *kernel_dv;
   
    switch(normalmap_params.filter)
    {
        case FILTER_SOBEL_3x3:
            kernel_size = FILTER_SOBEL_3x3_KERNEL_SIZE;
            kernel_du = FILTER_SOBEL_3x3_KERNEL_DU;
            kernel_dv = FILTER_SOBEL_3x3_KERNEL_DV;
            break;
        case FILTER_SOBEL_5x5:
            kernel_size = FILTER_SOBEL_5x5_KERNEL_SIZE;
            kernel_du = FILTER_SOBEL_5x5_KERNEL_DU;
            kernel_dv = FILTER_SOBEL_5x5_KERNEL_DV;
            break;
        case FILTER_PREWITT_3x3:
            kernel_size = FILTER_PREWITT_3x3_KERNEL_SIZE;
            kernel_du = FILTER_PREWITT_3x3_KERNEL_DU;
            kernel_dv = FILTER_PREWITT_3x3_KERNEL_DV;
            break;      
        case FILTER_PREWITT_5x5:
            kernel_size = FILTER_PREWITT_5x5_KERNEL_SIZE;
            kernel_du = FILTER_PREWITT_5x5_KERNEL_DU;
            kernel_dv = FILTER_PREWITT_5x5_KERNEL_DU;
            break;
        case FILTER_3x3:
            kernel_size = FILTER_3x3_KERNEL_SIZE;
            kernel_du = FILTER_3x3_KERNEL_DU;
            kernel_dv = FILTER_3x3_KERNEL_DV;
            break;
        case FILTER_5x5:
        {
            int n;
            float usum = 0, vsum = 0;
            float wt22 = 1.0f / 16.0f;
            float wt12 = 1.0f / 10.0f;
            float wt02 = 1.0f / 8.0f;
            float wt11 = 1.0f / 2.8f;
            kernel_size = 20;
            kernel_du = (kernel_element_t*)malloc(20 * sizeof(kernel_element_t));
            kernel_dv = (kernel_element_t*)malloc(20 * sizeof(kernel_element_t));
         
            kernel_du[0 ] = {-2,  2, -wt22};
            kernel_du[1 ] = {-1,  2, -wt12};
            kernel_du[2 ] = { 1,  2,  wt12};
            kernel_du[3 ] = { 2,  2,  wt22};
            kernel_du[4 ] = {-2,  1, -wt12};
            kernel_du[5 ] = {-1,  1, -wt11};
            kernel_du[6 ] = { 1,  1,  wt11};
            kernel_du[7 ] = { 2,  1,  wt12};
            kernel_du[8 ] = {-2,  0, -wt02};
            kernel_du[9 ] = {-1,  0, -0.5f};
            kernel_du[10] = { 1,  0,  0.5f};
            kernel_du[11] = { 2,  0,  wt02};
            kernel_du[12] = {-2, -1, -wt12};
            kernel_du[13] = {-1, -1, -wt11};
            kernel_du[14] = { 1, -1,  wt11};
            kernel_du[15] = { 2, -1,  wt12};
            kernel_du[16] = {-2, -2, -wt22};
            kernel_du[17] = {-1, -2, -wt12};
            kernel_du[18] = { 1, -2,  wt12};
            kernel_du[19] = { 2, -2,  wt22};
         
            kernel_dv[0 ] = {-2,  2,  wt22};
            kernel_dv[1 ] = {-1,  2,  wt12};
            kernel_dv[2 ] = { 0,  2, 0.25f};
            kernel_dv[3 ] = { 1,  2,  wt12};
            kernel_dv[4 ] = { 2,  2,  wt22};
            kernel_dv[5 ] = {-2,  1,  wt12};
            kernel_dv[6 ] = {-1,  1,  wt11};
            kernel_dv[7 ] = { 0,  1,  0.5f};
            kernel_dv[8 ] = { 1,  1,  wt11};
            kernel_dv[9 ] = { 2,  1,  wt22};
            kernel_dv[10] = {-2, -1, -wt22};
            kernel_dv[11] = {-1, -1, -wt11};
            kernel_dv[12] = { 0, -1, -0.5f};
            kernel_dv[13] = { 1, -1, -wt11};
            kernel_dv[14] = { 2, -1, -wt12};
            kernel_dv[15] = {-2, -2, -wt22};
            kernel_dv[16] = {-1, -2, -wt12};
            kernel_dv[17] = { 0, -2,-0.25f};
            kernel_dv[18] = { 1, -2, -wt12};
            kernel_dv[19] = { 2, -2, -wt22};

            for(n = 0; n < 20; ++n)
            {
               usum += fabsf(kernel_du[n].w);
               vsum += fabsf(kernel_dv[n].w);
            }
            for(n = 0; n < 20; ++n)
            {
               kernel_du[n].w /= usum;
               kernel_dv[n].w /= vsum;
            }
         
            break;
        }
        case FILTER_7x7:
            kernel_size = FILTER_7x7_KERNEL_SIZE;
            kernel_du = FILTER_7x7_KERNEL_DU;
            kernel_dv = FILTER_7x7_KERNEL_DV;
            break;
        case FILTER_9x9:
            kernel_size = FILTER_9x9_KERNEL_SIZE;
            kernel_du = FILTER_9x9_KERNEL_DU;
            kernel_dv = FILTER_9x9_KERNEL_DV;
            break;
        default:                                            // default is FILTER_SYM_DIFF
            kernel_size = FILTER_SYM_DIFF_KERNEL_SIZE;
            kernel_du = FILTER_SYM_DIFF_KERNEL_DU;
            kernel_dv = FILTER_SYM_DIFF_KERNEL_DV;
    }

    if(normalmap_params.conversion == CONVERT_BIASED_RGB)
    {
        // approximated average color of the image: scale to 16x16, accumulate the pixels and average
        unsigned int sum[3];

        tmp = (unsigned char*) malloc(16 * 16 * bpp);
        scale_pixels(tmp, 16, 16, rgba_src, width, height, bpp);

        sum[0] = sum[1] = sum[2] = 0;
      
        s = rgba_src;
        for(y = 0; y < 16; ++y)
        {
            for(x = 0; x < 16; ++x)
            {
                sum[0] += *s++;
                sum[1] += *s++;
                sum[2] += *s++;
                if (bpp == 4) s++;
            }
        }
         
        rgb_bias[0] = (float)sum[0] / 256.0f;
        rgb_bias[1] = (float)sum[1] / 256.0f;
        rgb_bias[2] = (float)sum[2] / 256.0f;
      
        free(tmp);
    }
    else
    {
        rgb_bias[0] = 0;
        rgb_bias[1] = 0;
        rgb_bias[2] = 0;
    }

    if(normalmap_params.conversion != CONVERT_NORMALIZE_ONLY && normalmap_params.conversion != CONVERT_DUDV_TO_NORMAL && normalmap_params.conversion != CONVERT_HEIGHTMAP)
    {
        s = rgba_src;
        for(y = 0; y < height; ++y)
        {
            for(x = 0; x < width; ++x)
            {
                if(!normalmap_params.height_source)
                {
                    switch(normalmap_params.conversion)
                    {
                        case CONVERT_NONE:
                            val = (float)s[0] * 0.3f + (float)s[1] * 0.59f + (float)s[2] * 0.11f;
                            break;
                        case CONVERT_BIASED_RGB:
                            val = (((float)glm::max(0.0f, s[0] - rgb_bias[0])) * 0.3f) + (((float)glm::max(0.0f, s[1] - rgb_bias[1])) * 0.59f) + (((float)glm::max(0.0f, s[2] - rgb_bias[2])) * 0.11f);
                            break;
                        case CONVERT_RED:
                            val = (float)s[0];
                            break;
                        case CONVERT_GREEN:
                            val = (float)s[1];
                            break;
                        case CONVERT_BLUE:
                            val = (float)s[2];
                            break;
                        case CONVERT_MAX_RGB:
                            val = (float)glm::max(s[0], glm::max(s[1], s[2]));
                            break;
                        case CONVERT_MIN_RGB:
                            val = (float)glm::min(s[0], glm::min(s[1], s[2]));
                            break;
                        case CONVERT_COLORSPACE:
                            val = (1.0f - ((1.0f - ((float)s[0] / 255.0f)) * (1.0f - ((float)s[1] / 255.0f)) * (1.0f - ((float)s[2] / 255.0f)))) * 255.0f;
                            break;
                        default:
                            val = 255.0f;
                            break;
                    }
                }
                else
                    val = (float)s[3];
         
                heights[x + y * width] = val * oneover255;
         
                s += bpp;
            }
        }
    }

#define HEIGHT(x,y) (heights[(glm::max(0, glm::min(width - 1, (x)))) + (glm::max(0, glm::min(height - 1, (y)))) * width])
#define HEIGHT_WRAP(x,y) (heights[((x) < 0 ? (width + (x)) : ((x) >= width ? ((x) - width) : (x))) + (((y) < 0 ? (height + (y)) : ((y) >= height ? ((y) - height) : (y))) * width)])

    for(y = 0; y < height; ++y)
    {
        for(x = 0; x < width; ++x)
        {
            d = rgba_dst + ((y * rowbytes) + (x * bpp));
            s = rgba_src + ((y * rowbytes) + (x * bpp));

            if(normalmap_params.conversion == CONVERT_NORMALIZE_ONLY || normalmap_params.conversion == CONVERT_HEIGHTMAP)
            {
                n[0] = (((float)s[0] * oneover255) - 0.5f) * 2.0f;
                n[1] = (((float)s[1] * oneover255) - 0.5f) * 2.0f;
                n[2] = (((float)s[2] * oneover255) - 0.5f) * 2.0f;
                n[0] *= normalmap_params.scale;
                n[1] *= normalmap_params.scale;
            }
            else if(normalmap_params.conversion == CONVERT_DUDV_TO_NORMAL)
            {
                n[0] = (((float)s[0] * oneover255) - 0.5f) * 2.0f;
                n[1] = (((float)s[1] * oneover255) - 0.5f) * 2.0f;
                n[2] = sqrtf(1.0f - (n[0] * n[0] - n[1] * n[1]));
                n[0] *= normalmap_params.scale;
                n[1] *= normalmap_params.scale;
            }
            else
            {
                du = 0; dv = 0;
                if(!normalmap_params.wrap)
                {
                    for(i = 0; i < num_elements; ++i)
                        du += HEIGHT(x + kernel_du[i].x, y + kernel_du[i].y) * kernel_du[i].w;
                    for(i = 0; i < num_elements; ++i)
                        dv += HEIGHT(x + kernel_dv[i].x, y + kernel_dv[i].y) * kernel_dv[i].w;
                }
                else
                {
                    for(i = 0; i < num_elements; ++i) du += HEIGHT_WRAP(x + kernel_du[i].x, y + kernel_du[i].y) * kernel_du[i].w;
                    for(i = 0; i < num_elements; ++i) dv += HEIGHT_WRAP(x + kernel_dv[i].x, y + kernel_dv[i].y) * kernel_dv[i].w;
                }
            
                n[0] = -du * normalmap_params.scale;
                n[1] = -dv * normalmap_params.scale;
                n[2] = 1.0f;
            }

            NORMALIZE(n);
         
            if(n[2] < normalmap_params.minz)
            {
                n[2] = normalmap_params.minz;
                NORMALIZE(n);
            }


            if(normalmap_params.xinvert) n[0] = -n[0];
            if(normalmap_params.yinvert) n[1] = -n[1];
            if(normalmap_params.swapRGB)
            {
                val = n[0];
                n[0] = n[2];
                n[2] = val;
            }            
         
            if(!normalmap_params.dudv)
            {
                *d++ = (unsigned char)((n[0] + 1.0f) * 127.5f);
                *d++ = (unsigned char)((n[1] + 1.0f) * 127.5f);
                *d++ = (unsigned char)((n[2] + 1.0f) * 127.5f);
         
                switch(normalmap_params.alpha)
                {
                    case ALPHA_NONE:           *d++ = s[3]; break;
                    case ALPHA_HEIGHT:         *d++ = (unsigned char)(heights[x + y * width] * 255.0f); break;
                    case ALPHA_INVERSE_HEIGHT: *d++ = 255 - (unsigned char)(heights[x + y * width] * 255.0f); break;
                    case ALPHA_ZERO:           *d++ = 0; break;
                    case ALPHA_ONE:            *d++ = 255; break;
                    case ALPHA_INVERT:         *d++ = 255 - s[3]; break;
                    default:                   *d++ = s[3]; break;
                }
            }
            else
            {
                if(normalmap_params.dudv == DUDV_8BIT_SIGNED || normalmap_params.dudv == DUDV_8BIT_UNSIGNED)
                {
                    if(normalmap_params.dudv == DUDV_8BIT_UNSIGNED)
                    {
                        n[0] += 1.0f;
                        n[1] += 1.0f;
                    }
                    *d++ = (unsigned char)(n[0] * 127.5f);
                    *d++ = (unsigned char)(n[1] * 127.5f);
                    *d++ = 0;
                }
                else if(normalmap_params.dudv == DUDV_16BIT_SIGNED || normalmap_params.dudv == DUDV_16BIT_UNSIGNED)
                {
                    unsigned short *d16 = (unsigned short*)d;
                    if(normalmap_params.dudv == DUDV_16BIT_UNSIGNED)
                    {
                        n[0] += 1.0f;
                        n[1] += 1.0f;
                    }
                    *d16++ = (unsigned short)(n[0] * 32767.5f);
                    *d16++ = (unsigned short)(n[1] * 32767.5f);
                }
            }
        }
    }

    if(normalmap_params.conversion == CONVERT_HEIGHTMAP)
        make_heightmap(rgba_dst, width, height, bpp, normalmap_params);

#undef HEIGHT
#undef HEIGHT_WRAP

    free(heights);
}