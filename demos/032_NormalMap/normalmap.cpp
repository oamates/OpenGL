#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include "normalmap.hpp"

#include <glm/glm.hpp>

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

template<typename real_t> void normalmap(unsigned char* src, unsigned char* dst, int width, int height, int bpp, const normalmap_params_t& normalmap_params)
{
    const real_t one = 1.0;
    const real_t inv_255 = 1.0 / 255.0;
    const real_t inv_127_5 = 2.0 / 255.0;
    const real_t inv_32767_5 = 2.0 / 65535.0;
    const real_t scale = (real_t) normalmap_params.scale;
    const glm::tvec3<real_t> rgb_power = glm::tvec3<real_t>(0.299, 0.587, 0.114);
    const real_t max_byte = 255.0;

    int area = width * height;
    int rowbytes = width * bpp;

    int kernel_size;
    kernel_element_t* kernel;
    real_t normalization_factor;

    real_t* heights = (real_t*) calloc(width * height, sizeof(real_t));
   
    switch(normalmap_params.filter)
    {
        case FILTER_SOBEL_3x3:
            kernel_size = FILTER_SOBEL_3x3_KERNEL_SIZE;
            kernel = FILTER_SOBEL_3x3_KERNEL;
            normalization_factor = FILTER_SOBEL_3x3_NORMALIZATION_FACTOR;
            break;
        case FILTER_SOBEL_5x5:
            kernel_size = FILTER_SOBEL_5x5_KERNEL_SIZE;
            kernel = FILTER_SOBEL_5x5_KERNEL;
            normalization_factor = FILTER_SOBEL_5x5_NORMALIZATION_FACTOR;
            break;
        case FILTER_PREWITT_3x3:
            kernel_size = FILTER_PREWITT_3x3_KERNEL_SIZE;
            kernel = FILTER_PREWITT_3x3_KERNEL;
            normalization_factor = FILTER_PREWITT_5x5_NORMALIZATION_FACTOR;
            break;
        case FILTER_PREWITT_5x5:
            kernel_size = FILTER_PREWITT_5x5_KERNEL_SIZE;
            kernel = FILTER_PREWITT_5x5_KERNEL;
            normalization_factor = FILTER_PREWITT_5x5_NORMALIZATION_FACTOR;
            break;
        case FILTER_3x3:
            kernel_size = FILTER_3x3_KERNEL_SIZE;
            kernel = FILTER_3x3_KERNEL;
            normalization_factor = FILTER_3x3_NORMALIZATION_FACTOR;
            break;
        case FILTER_5x5:
            kernel_size = FILTER_5x5_KERNEL_SIZE;
            kernel = FILTER_5x5_KERNEL;
            normalization_factor = FILTER_5x5_NORMALIZATION_FACTOR;
            break;
        case FILTER_7x7:
            kernel_size = FILTER_7x7_KERNEL_SIZE;
            kernel = FILTER_7x7_KERNEL;
            normalization_factor = FILTER_7x7_NORMALIZATION_FACTOR;
            break;
        case FILTER_9x9:
            kernel_size = FILTER_9x9_KERNEL_SIZE;
            kernel = FILTER_9x9_KERNEL;
            normalization_factor = FILTER_9x9_NORMALIZATION_FACTOR;
            break;
        default:                                            // default is FILTER_SYM_DIFF
            kernel_size = FILTER_SYM_DIFF_KERNEL_SIZE;
            kernel = FILTER_SYM_DIFF_KERNEL;
            normalization_factor = FILTER_SYM_DIFF_NORMALIZATION_FACTOR;
    }

    glm::tvec3<real_t> rgb_bias = glm::tvec3<real_t>(real_t(0.0));
    if(normalmap_params.conversion == CONVERT_BIASED_RGB)   // compute average color of the image and use it as bias
    {
        unsigned char* s = src;
        for(int i = 0; i < area; ++i)
        {
            glm::tvec3<real_t> rgb = glm::tvec3<real_t>(s[0], s[1], s[2]);
            rgb_bias += rgb;
            s += bpp;
        }
        real_t inv_area = 1.0 / area;
        rgb_bias *= inv_area;
    }

    if(normalmap_params.conversion != CONVERT_NORMALIZE_ONLY && 
       normalmap_params.conversion != CONVERT_DUDV_TO_NORMAL && 
       normalmap_params.conversion != CONVERT_HEIGHTMAP)
    {
        unsigned char* s = src;
        for(int i = 0; i < area; ++i)
        {
            real_t val;
            if(!normalmap_params.height_source)
            {
                glm::tvec3<real_t> rgb = inv_255 * glm::tvec3<real_t>(s[0], s[1], s[2]);
                switch(normalmap_params.conversion)
                {
                    case CONVERT_NONE:       val = glm::dot(rgb_power, rgb); break;
                    case CONVERT_BIASED_RGB: val = glm::dot(rgb_power, glm::max(glm::tvec3<real_t>(0.0), rgb - rgb_bias)); break;
                    case CONVERT_RED:        val = rgb.r; break;
                    case CONVERT_GREEN:      val = rgb.g; break;
                    case CONVERT_BLUE:       val = rgb.b; break;
                    case CONVERT_MAX_RGB:    val = glm::max(rgb.r, glm::max(rgb.g, rgb.b)); break;
                    case CONVERT_MIN_RGB:    val = glm::min(rgb.r, glm::min(rgb.g, rgb.b)); break;
                    case CONVERT_COLORSPACE: val = one - (one - rgb.r) * (one - rgb.g) * (one - rgb.b); break;
                    default: val = one;
                }
            }
            else
                val = inv_255 * s[3];
         
            heights[i] = val;
            s += bpp;
        }
    }

#define HEIGHT(x,y) (heights[(glm::max(0, glm::min(width - 1, (x)))) + (glm::max(0, glm::min(height - 1, (y)))) * width])
#define HEIGHT_WRAP(x,y) (heights[((x) < 0 ? (width + (x)) : ((x) >= width ? ((x) - width) : (x))) + (((y) < 0 ? (height + (y)) : ((y) >= height ? ((y) - height) : (y))) * width)])

    int idx = 0;
    for(int y = 0; y < height; ++y)
    {
        for(int x = 0; x < width; ++x)
        {
            unsigned char* d = dst + 4 * idx;
            unsigned char* s = src + bpp * idx;

            glm::tvec3<real_t> n;
            if(normalmap_params.conversion == CONVERT_NORMALIZE_ONLY || normalmap_params.conversion == CONVERT_HEIGHTMAP)
            {
                n = glm::tvec3<real_t>(inv_127_5 * s[0] - one, inv_127_5 * s[1] - one, inv_127_5 * s[2] - one);
                n.x *= scale;
                n.y *= scale;
            }
            else if(normalmap_params.conversion == CONVERT_DUDV_TO_NORMAL)
            {
                n.x = inv_127_5 * s[0] - one;
                n.y = inv_127_5 * s[1] - one;
                n.z = glm::sqrt(one - (n.x * n.x + n.y * n.y));
                n.x *= scale;
                n.y *= scale;
            }
            else
            {
                real_t du = 0.0; 
                real_t dv = 0.0;
                if(!normalmap_params.wrap)
                {
                    for(int i = 0; i < kernel_size; ++i)
                    {
                        real_t w = real_t(kernel[i].w);
                        du += HEIGHT(x + kernel[i].x, y + kernel[i].y) * w;
                        dv += HEIGHT(x + kernel[i].y, y + kernel[i].x) * w;
                    }
                }
                else
                {
                    for(int i = 0; i < kernel_size; ++i)
                    {
                        real_t w = real_t(kernel[i].w);
                        du += HEIGHT_WRAP(x + kernel[i].x, y + kernel[i].y) * w;
                        dv += HEIGHT_WRAP(x + kernel[i].y, y + kernel[i].x) * w;
                    }
                }
            
                n = glm::vec3(-du * scale * normalization_factor,
                              -dv * scale * normalization_factor, one);
            }

            n = glm::normalize(n);
         
            if(n.z < normalmap_params.minz)
            {
                n.z = normalmap_params.minz;
                n = glm::normalize(n);
            }

            if(normalmap_params.xinvert) n.x = -n.x;
            if(normalmap_params.yinvert) n.y = -n.y;          
         
            if(normalmap_params.dudv == DUDV_NONE)
            {
                *d++ = inv_127_5 * (n.x + one);
                *d++ = inv_127_5 * (n.y + one);
                *d++ = inv_127_5 * (n.z + one);
         
                switch(normalmap_params.alpha)
                {
                    case ALPHA_HEIGHT:         *d++ = (unsigned char)(heights[x + y * width] * max_byte); break;
                    case ALPHA_INVERSE_HEIGHT: *d++ = 255 - (unsigned char)(heights[x + y * width] * max_byte); break;
                    case ALPHA_ZERO:           *d++ = 0; break;
                    case ALPHA_ONE:            *d++ = 255; break;
                    case ALPHA_INVERT:         *d++ = 255 - s[3]; break;
                    default:                   *d++ = s[3];
                }
            }
            else
            {
                if(normalmap_params.dudv == DUDV_8BIT_SIGNED || normalmap_params.dudv == DUDV_8BIT_UNSIGNED)
                {
                    if(normalmap_params.dudv == DUDV_8BIT_UNSIGNED)
                    {
                        n.x += one;
                        n.y += one;
                    }
                    *d++ = inv_127_5 * n.x;
                    *d++ = inv_127_5 * n.y;
                    *d++ = 0;
                }
                else if(normalmap_params.dudv == DUDV_16BIT_SIGNED || normalmap_params.dudv == DUDV_16BIT_UNSIGNED)
                {
                    unsigned short* d16 = (unsigned short*) d;
                    if(normalmap_params.dudv == DUDV_16BIT_UNSIGNED)
                    {
                        n.x += one;
                        n.y += one;
                    }
                    *d16++ = inv_32767_5 * n.x;
                    *d16++ = inv_32767_5 * n.y;
                }
            }
            idx++;
        }
    }

/*
    if(normalmap_params.conversion == CONVERT_HEIGHTMAP)
        make_heightmap(rgba_dst, width, height, bpp, normalmap_params);
*/
#undef HEIGHT
#undef HEIGHT_WRAP

    free(heights);
}

template void normalmap<float> (unsigned char*, unsigned char*, int, int, int, const normalmap_params_t&);
template void normalmap<double>(unsigned char*, unsigned char*, int, int, int, const normalmap_params_t&);