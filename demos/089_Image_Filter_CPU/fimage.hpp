#ifndef _fimage_included_128936817254712541725497315482154120545612745618729356
#define _fimage_included_128936817254712541725497315482154120545612745618729356

#include "image.hpp"

//#include "nvmath/nvmath.h" // lerp

//#include "nvcore/Debug.h"
//#include "nvcore/Utils.h" // clamp

#include <cstdlib> // abs

namespace nv
{
    class Vector4;
    class Matrix;
    class Image;
    class Filter;
    class Kernel1;
    class Kernel2;
    class PolyphaseKernel;

    //===================================================================================================================================================================================================================
    // Multicomponent floating point image class.
    //===================================================================================================================================================================================================================
    struct fimage_t
    {
        uint16_t m_componentCount;
        uint16_t m_width;
        uint16_t m_height;
        uint16_t m_depth;
        uint32_t m_pixelCount;
        uint32_t m_floatCount;
        float* m_mem;

        enum WrapMode
        {
            WrapMode_Clamp,
            WrapMode_Repeat,
            WrapMode_Mirror
        };

        fimage_t() : m_componentCount(0), m_width(0), m_height(0), m_depth(0), m_pixelCount(0), m_floatCount(0), m_mem(0) {}

        fimage_t(const image_t* image) : m_componentCount(0), m_width(0), m_height(0), m_depth(0), m_pixelCount(0), m_floatCount(0), m_mem(0)
            { initFrom(image); }
        
        virtual ~fimage_t()
            { free(); }

        void initFrom(const image* img)
        {
            allocate(4, img->width(), img->height(), img->depth());

            float* red_channel   = channel(0);
            float* green_channel = channel(1);
            float* blue_channel  = channel(2);
            float* alpha_channel = channel(3);

            const uint count = m_pixelCount;
            for (uint i = 0; i < count; i++)
            {
                Color32 pixel = img->pixel(i);
                red_channel[i]   = float(pixel.r) / 255.0f;
                green_channel[i] = float(pixel.g) / 255.0f;
                blue_channel[i]  = float(pixel.b) / 255.0f;
                alpha_channel[i] = float(pixel.a) / 255.0f;
            }
        }

        image_t* createImage(uint base_component = 0, uint num = 4) const
        {
            image_t* image = new image_t();
            img->allocate(m_width, m_height, m_depth);

            for (uint i = 0; i < m_pixelCount; i++)
            {
                uint8_t rgba[4]= {0, 0, 0, 0xff};

                for (unsigned int c = 0; c < num; c++)
                {
                    float f = pixel(baseComponent + c, i);
                    rgba[c] = nv::clamp(int(255.0f * f), 0, 255);
                }

                image->pixel(i) = Color32(rgba[0], rgba[1], rgba[2], rgba[3]);
            }
            return image;
        }

        //===============================================================================================================================================================================================================
        // converts the floating point image to a regular image, corrects gamma of rgb, but not alpha
        //===============================================================================================================================================================================================================
        image_t* fimage_t::createImageGammaCorrect(float gamma = 2.2f) const
        {

            image_t* image = new image_t();
            image->allocate(m_width, m_height, m_depth);

            const float* rChannel = this->channel(0);
            const float* gChannel = this->channel(1);
            const float* bChannel = this->channel(2);
            const float* aChannel = this->channel(3);

            const uint count = m_pixelCount;
            float inv_gamma = 1.0f / gamma;
            for (uint i = 0; i < count; i++)
            {
                const uint8 r = nv::clamp(int(255.0f * pow(rChannel[i], inv_gamma)), 0, 255);
                const uint8 g = nv::clamp(int(255.0f * pow(gChannel[i], inv_gamma)), 0, 255);
                const uint8 b = nv::clamp(int(255.0f * pow(bChannel[i], inv_gamma)), 0, 255);
                const uint8 a = nv::clamp(int(255.0f * aChannel[i]), 0, 255);

                image->pixel(i) = Color32(r, g, b, a);
            }

            return image;
        }

        //===============================================================================================================================================================================================================
        // allocates a 2D float image of the given format and the given extents.
        //===============================================================================================================================================================================================================
        void allocate(uint c, uint w, uint h, uint d = 1)
        {   
            if (m_componentCount != c || m_width != w || m_height != h || m_depth != d)
            {
                free();

                m_width = w;
                m_height = h;
                m_depth = d;
                m_componentCount = c;
                m_pixelCount = w * h * d;
                m_floatCount = m_pixelCount * c;
                m_mem = malloc<float>(m_floatCount);
            }
        }

        void free()
        {
            ::free(m_mem);
            m_mem = 0;
        }

        void resizeChannelCount(uint c)
        {
            if (m_componentCount != c)
            {
                uint count = m_pixelCount * c;
                m_mem = realloc<float>(m_mem, count);

                if (c > m_componentCount)
                    memset(m_mem + m_floatCount, 0, (count - m_floatCount) * sizeof(float));

                m_componentCount = c;
                m_floatCount = count;
            }
        }

        void clear(float f = 0.0f)
        {
            for (uint i = 0; i < m_floatCount; i++) m_mem[i] = f;
        }

        void clear(uint component, float f = 0.0f)
        {
            float* channel = this->channel(c);

            const uint count = m_pixelCount;
            for (uint i = 0; i < count; i++)
                channel[i] = f;
        }

        void copyChannel(uint src, uint dst)
        {
            const float * srcChannel = this->channel(src);
            float * dstChannel = this->channel(dst);
            memcpy(dstChannel, srcChannel, sizeof(float) * m_pixelCount);
        }

        void normalize(uint base_component);

        void packNormals(uint base_component);
        void expandNormals(uint base_component);
        void scaleBias(uint base_component, uint num, float scale, float add);

        // Clamps the elements of the image
        void clamp(uint base_component, uint num, float low, float high)
        {
            for (uint c = 0; c < num; c++)
            {
                float* ptr = this->channel(base_component + c);
                for (uint i = 0; i < m_pixelCount; i++)
                    ptr[i] = nv::clamp(ptr[i], low, high);
            }
        }

        //===============================================================================================================================================================================================================
        // From gamma space to linear space
        //===============================================================================================================================================================================================================
        void toLinear(uint base_component, uint num, float gamma = 2.2f);
            { exponentiate(base_component, num, gamma); }

        //===============================================================================================================================================================================================================
        // From linear to gamma space
        //===============================================================================================================================================================================================================
        void toGamma(uint base_component, uint num, float gamma = 2.2f);
            { exponentiate(base_component, num, 1.0f / gamma); }

        //===============================================================================================================================================================================================================
        // Exponentiate the elements of the image
        //===============================================================================================================================================================================================================
        void exponentiate(uint base_component, uint num, float power)
        {
            for(uint c = 0; c < num; c++)
            {
                float * ptr = this->channel(base_component + c);

                for(uint i = 0; i < m_pixelCount; i++)
                    ptr[i] = powf(max(0.0f, ptr[i]), power);
            }
        }

        void transform(uint base_component, const Matrix & m, const Vector4 & offset);
        void swizzle(uint base_component, uint r, uint g, uint b, uint a);

        fimage_t* fastDownSample() const;
        fimage_t* downSample(const Filter & filter, WrapMode wm) const;
        fimage_t* downSample(const Filter & filter, WrapMode wm, uint alpha) const;
        fimage_t* resize(const Filter & filter, uint w, uint h, WrapMode wm) const;
        fimage_t* resize(const Filter & filter, uint w, uint h, uint d, WrapMode wm) const;
        fimage_t* resize(const Filter & filter, uint w, uint h, WrapMode wm, uint alpha) const;
        fimage_t* resize(const Filter & filter, uint w, uint h, uint d, WrapMode wm, uint alpha) const;

        void convolve(const Kernel2 & k, uint c, WrapMode wm);

        float applyKernelXY(const Kernel2 * k, int x, int y, int z, uint c, WrapMode wm) const;
        float applyKernelX(const Kernel1 * k, int x, int y, int z, uint c, WrapMode wm) const;
        float applyKernelY(const Kernel1 * k, int x, int y, int z, uint c, WrapMode wm) const;
        float applyKernelZ(const Kernel1 * k, int x, int y, int z, uint c, WrapMode wm) const;
        void applyKernelX(const PolyphaseKernel& k, int y, int z, uint c, WrapMode wm, float* output) const;
        void applyKernelY(const PolyphaseKernel& k, int x, int z, uint c, WrapMode wm, float* output) const;
        void applyKernelZ(const PolyphaseKernel& k, int x, int y, uint c, WrapMode wm, float* output) const;
        void applyKernelX(const PolyphaseKernel& k, int y, int z, uint c, uint a, WrapMode wm, float* output) const;
        void applyKernelY(const PolyphaseKernel& k, int x, int z, uint c, uint a, WrapMode wm, float* output) const;

        //===============================================================================================================================================================================================================
        // Apply 1D horizontal kernel at the given coordinates and return result.
        //===============================================================================================================================================================================================================
        void applyKernelZ(const PolyphaseKernel & k, int x, int y, uint c, uint a, WrapMode wm, float * __restrict output) const
        {
            const uint length = k.length();
            const float scale = float(length) / float(m_width);
            const float iscale = 1.0f / scale;

            const float width = k.width();
            const int windowSize = k.windowSize();

            const float* channel = this->channel(c);
            const float* alpha = this->channel(a);

            for (uint i = 0; i < length; i++)
            {
                const float center = (0.5f + i) * iscale;

                const int left = (int)floorf(center - width);
                const int right = (int)ceilf(center + width);

                float norm = 0.0f;
                float sum = 0;
                for (int j = 0; j < windowSize; ++j)
                {
                    const int idx = this->index(x, y, left + j, wm);

                    float w = k.valueAt(i, j) * (alpha[idx] + (1.0f / 256.0f));
                    norm += w;
                    sum += w * channel[idx];
                }

                output[i] = sum / norm;
            }
        }

        void flip_x()
        {
            const unsigned int w2 = m_width / 2;

            for (unsigned int c = 0; c < m_componentCount; c++)
                for (unsigned int z = 0; z < m_depth; z++)
                    for (unsigned int y = 0; y < m_height; y++)
                    {
                        float* line = scanline(c, y, z);
                        for (unsigned int x = 0; x < w2; x++)
                            std::swap(line[x], line[w - 1 - x]);
                    }
        }

        void flip_y()
        {
            const unsigned int h2 = m_height / 2;

            for (unsigned int c = 0; c < m_componentCount; c++)
                for (unsigned int z = 0; z < m_depth; z++)
                    for (unsigned int y = 0; y < h2; y++)
                    {
                        float* src = scanline(c, y, z);
                        float* dst = scanline(c, m_height - 1 - y, z);
                        for (unsigned int x = 0; x < m_width; x++)
                            std::swap(src[x], dst[x]);
                    }
        }

        void flip_z()
        {
            const unsigned int d2 = m_depth / 2;

            for (unsigned int c = 0; c < m_componentCount; c++)
                for (unsigned int z = 0; z < d2; z++)
                {
                    float* src = plane(c, z);
                    float* dst = plane(c, m_depth - 1 - z);
                    for (unsigned int i = 0; i < m_width * m_height; i++)
                        std::swap(src[i], dst[i]);
                }
        }        

        float alphaTestCoverage(float alphaRef, int alphaChannel, float alphaScale = 1.0f) const
        {
            const uint w = m_width;
            const uint h = m_height;


            const uint n = 8;
            float coverage = 0.0f;

            for (uint y = 0; y < h - 1; y++)
            {
                for (uint x = 0; x < w - 1; x++)
                {

                    float alpha00 = nv::saturate(pixel(alphaChannel, x + 0, y + 0, 0) * alphaScale);
                    float alpha10 = nv::saturate(pixel(alphaChannel, x + 1, y + 0, 0) * alphaScale);
                    float alpha01 = nv::saturate(pixel(alphaChannel, x + 0, y + 1, 0) * alphaScale);
                    float alpha11 = nv::saturate(pixel(alphaChannel, x + 1, y + 1, 0) * alphaScale);

                    for (float fy = 0.5f / n; fy < 1.0f; fy++)
                    {
                        for (float fx = 0.5f / n; fx < 1.0f; fx++)
                        {
                            float alpha = alpha00 * (1 - fx) * (1 - fy) + alpha10 * fx * (1 - fy) + alpha01 * (1 - fx) * fy + alpha11 * fx * fy;
                            if (alpha > alphaRef) coverage += 1.0f;
                        }
                    }
                }
            }
            return coverage / float(w * h * n * n);
        }

        void scaleAlphaToCoverage(float desiredCoverage, float alphaRef, int alphaChannel)
        {
            float minAlphaScale = 0.0f;
            float maxAlphaScale = 4.0f;
            float alphaScale = 1.0f;

            // Determine desired scale using a binary search. Hardcoded to 8 steps max.
            for (int i = 0; i < 10; i++)
            {
                float currentCoverage = alphaTestCoverage(alphaRef, alphaChannel, alphaScale);

                if (currentCoverage < desiredCoverage) {
                    minAlphaScale = alphaScale;
                }
                else if (currentCoverage > desiredCoverage) {
                    maxAlphaScale = alphaScale;
                }
                else break;

                alphaScale = (minAlphaScale + maxAlphaScale) * 0.5f;
            }

            // Scale alpha channel.
            scaleBias(alphaChannel, 1, alphaScale, 0.0f);
            clamp(alphaChannel, 1, 0.0f, 1.0f); 
        }

        uint width() const { return m_width; }
        uint height() const { return m_height; }
        uint depth() const { return m_depth; }
        uint componentCount() const { return m_componentCount; }
        uint floatCount() const { return m_floatCount; }
        uint pixelCount() const { return m_pixelCount; }

        const float* channel(uint c) const
            { return m_mem + c * m_pixelCount; }

        float* channel(uint c)
            { return m_mem + c * m_pixelCount; }

        const float* plane(uint c, uint z) const
            { return channel(c) + z * m_width * m_height; }

        float* plane(uint c, uint z);
            { return channel(c) + z * m_width * m_height; }

        const float* scanline(uint c, uint y, uint z) const
            { return plane(c, z) + y * m_width; }

        float* scanline(uint c, uint y, uint z)
            { return plane(c, z) + y * m_width; }

        float pixel(uint c, uint x, uint y, uint z) const
            { return m_mem[c * m_pixelCount + index(x, y, z)]; }

        float& pixel(uint c, uint x, uint y, uint z)
            { return m_mem[c * m_pixelCount + index(x, y, z)]; }

        float pixel(uint c, uint idx) const
            { return m_mem[c * m_pixelCount + idx]; }

        float& pixel(uint c, uint idx)
            { return m_mem[c * m_pixelCount + idx]; }

        float pixel(uint idx) const
            { return m_mem[idx]; }
        float& pixel(uint idx);
            { return m_mem[idx]; }








































        float sampleNearest(uint c, float x, float y, WrapMode wm) const;
        float sampleLinear(uint c, float x, float y, WrapMode wm) const;

        float sampleNearest(uint c, float x, float y, float z, WrapMode wm) const;
        float sampleLinear(uint c, float x, float y, float z, WrapMode wm) const;

        float sampleNearestClamp(uint c, float x, float y) const;
        float sampleNearestRepeat(uint c, float x, float y) const;
        float sampleNearestMirror(uint c, float x, float y) const;

        float sampleNearestClamp(uint c, float x, float y, float z) const;
        float sampleNearestRepeat(uint c, float x, float y, float z) const;
        float sampleNearestMirror(uint c, float x, float y, float z) const;

        float sampleLinearClamp(uint c, float x, float y) const;
        float sampleLinearRepeat(uint c, float x, float y) const;
        float sampleLinearMirror(uint c, float x, float y) const;

        float sampleLinearClamp(uint c, float x, float y, float z) const;
        float sampleLinearRepeat(uint c, float x, float y, float z) const;
        float sampleLinearMirror(uint c, float x, float y, float z) const;

        fimage_t* clone() const
        {
            fimage_t* copy = new fimage_t();
            copy->allocate(m_componentCount, m_width, m_height, m_depth);
            memcpy(copy->m_mem, m_mem, m_floatCount * sizeof(float));
            return copy;
        }

        uint index(uint x, uint y, uint z) const
        {
            uint idx = (z * m_height + y) * m_width + x;
            return idx;
        }

        uint indexClamp(int x, int y, int z) const
        {
            x = wrapClamp(x, m_width);
            y = wrapClamp(y, m_height);
            z = wrapClamp(z, m_depth);
            return index(x, y, z);
        }


        uint indexRepeat(int x, int y, int z) const
        {
            x = wrapRepeat(x, m_width);
            y = wrapRepeat(y, m_height);
            z = wrapRepeat(z, m_depth);
            return index(x, y, z);
        }

        uint indexMirror(int x, int y, int z) const
        {
            x = wrapMirror(x, m_width);
            y = wrapMirror(y, m_height);
            z = wrapMirror(z, m_depth);
            return index(x, y, z);
        }

        uint index(int x, int y, int z, WrapMode wm) const
        {
            if (wm == WrapMode_Clamp)  return indexClamp(x, y, z);
            if (wm == WrapMode_Repeat) return indexRepeat(x, y, z);
            return indexMirror(x, y, z);
        }

        float bilerp(uint c, int ix0, int iy0, int ix1, int iy1, float fx, float fy) const;
        {
            int iz = 0;
            float f1 = pixel(c, ix0, iy0, iz);
            float f2 = pixel(c, ix1, iy0, iz);
            float f3 = pixel(c, ix0, iy1, iz);
            float f4 = pixel(c, ix1, iy1, iz);

            float i1 = lerp(f1, f2, fx);
            float i2 = lerp(f3, f4, fx);

            return lerp(i1, i2, fy);
        }

        float trilerp(uint c, int ix0, int iy0, int iz0, int ix1, int iy1, int iz1, float fx, float fy, float fz) const;
        {
            float f000 = pixel(c, ix0, iy0, iz0);
            float f100 = pixel(c, ix1, iy0, iz0);
            float f010 = pixel(c, ix0, iy1, iz0);
            float f110 = pixel(c, ix1, iy1, iz0);
            float f001 = pixel(c, ix0, iy0, iz1);
            float f101 = pixel(c, ix1, iy0, iz1);
            float f011 = pixel(c, ix0, iy1, iz1);
            float f111 = pixel(c, ix1, iy1, iz1);

            float i1 = lerp(f000, f001, fz);
            float i2 = lerp(f010, f011, fz);
            float j1 = lerp(f100, f101, fz);
            float j2 = lerp(f110, f111, fz);

            float w1 = lerp(i1, i2, fy);
            float w2 = lerp(j1, j2, fy);

            return lerp(w1, w2, fx);
        }
    };

    inline int wrapClamp(int x, int w)
    {
        return nv::clamp(x, 0, w - 1);
    }

    inline int wrapRepeat(int x, int w)
    {
        if (x >= 0) return x % w;
        else return (x + 1) % w + w - 1;
    }

    inline int wrapMirror(int x, int w)
    {
        if (w == 1) x = 0;

        x = abs(x);
        while (x >= w) {
            x = abs(w + w - x - 2);
        }

        return x;
    }

    inline bool sameLayout(const fimage_t* img0, const fimage_t* img1)
    {
        return img0->width() == img1->width() && img0->height() == img1->height() && img0->depth() == img1->depth();
    }


} // nv namespace

#endif // _fimage_included_128936817254712541725497315482154120545612745618729356