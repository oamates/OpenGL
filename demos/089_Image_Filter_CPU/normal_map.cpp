#include "normal_map.hpp"
#include "filter.hpp"
#include "fimage.hpp"
#include "image.hpp"

//#include "nvmath/Color.inl"
//#include "nvmath/Vector.h"

//#include "nvcore/Ptr.h"

#include <cstring>


using namespace nv;

static fimage_t* createNormalMap(const Image * img, fimage_t::WrapMode wm, Vector4::Arg heightWeights, const Kernel2 * kdu, const Kernel2 * kdv)
{
    const uint w = img->width();
    const uint h = img->height();

    AutoPtr<fimage_t> fimage(new fimage_t());
    fimage->allocate(4, w, h);

    // Compute height and store in alpha channel:
    float * alphaChannel = fimage->channel(3);
    for(uint i = 0; i < w * h; i++)
    {
        Vector4 color = toVector4(img->pixel(i));
        alphaChannel[i] = dot(color, heightWeights);
    }

    float heightScale = 1.0f / 16.0f;

    for(uint y = 0; y < h; y++)
    {
        for(uint x = 0; x < w; x++)
        {
            const float du = fimage->applyKernelXY(kdu, x, y, 0, 3, wm);
            const float dv = fimage->applyKernelXY(kdv, x, y, 0, 3, wm);

            Vector3 n = normalize(Vector3(du, dv, heightScale));

            fimage->pixel(0, x, y, 0) = 0.5f * n.x + 0.5f;
            fimage->pixel(1, x, y, 0) = 0.5f * n.y + 0.5f;
            fimage->pixel(2, x, y, 0) = 0.5f * n.z + 0.5f;
        }
    }

    return fimage.release();
}


//========================================================================================================================================================================================================================
// Create normal map using the given kernels.
//========================================================================================================================================================================================================================
static fimage_t* createNormalMap(const fimage_t * img, fimage_t::WrapMode wm, const Kernel2 * kdu, const Kernel2 * kdv)
{
    nvDebugCheck(kdu != NULL);
    nvDebugCheck(kdv != NULL);
    nvDebugCheck(img != NULL);

    const float heightScale = 1.0f / 16.0f;

    const uint w = img->width();
    const uint h = img->height();

    AutoPtr<fimage_t> img_out(new fimage_t());
    img_out->allocate(4, w, h);

    for (uint y = 0; y < h; y++)
    {
        for (uint x = 0; x < w; x++)
        {
            const float du = img->applyKernelXY(kdu, x, y, 0, 3, wm);
            const float dv = img->applyKernelXY(kdv, x, y, 0, 3, wm);

            Vector3 n = normalize(Vector3(du, dv, heightScale));

            img_out->pixel(0, x, y, 0) = n.x;
            img_out->pixel(1, x, y, 0) = n.y;
            img_out->pixel(2, x, y, 0) = n.z;
        }
    }

    // Copy alpha channel.
    /*for (uint y = 0; y < h; y++)
    {
        for (uint x = 0; x < w; x++)
        {
            
            img_out->pixel(3, x, y, 0) = img->pixel(3, x, y, 0);
        }
    }*/
    memcpy(img_out->channel(3), img->channel(3), w * h * sizeof(float));

    return img_out.release();
}


//========================================================================================================================================================================================================================
// Create normal map using the given filter.
//========================================================================================================================================================================================================================
fimage_t * nv::createNormalMap(const Image * img, fimage_t::WrapMode wm, Vector4::Arg heightWeights, NormalMapFilter filter /*= Sobel3x3*/)
{
    nvDebugCheck(img != NULL);

    // Init the kernels.
    Kernel2* kdu = (filter == NormalMapFilter_Sobel3x3) ? new Kernel2(3) :
                   (filter == NormalMapFilter_Sobel5x5) ? new Kernel2(5) :
                   (filter == NormalMapFilter_Sobel7x7) ? new Kernel2(7) : new Kernel2(9);

    kdu->initSobel();
    kdu->normalize();

    Kernel2 * kdv = new Kernel2(*kdu);
    kdv->transpose();

    return ::createNormalMap(img, wm, heightWeights, kdu, kdv);
}


//========================================================================================================================================================================================================================
// Create normal map combining multiple sobel filters.
//========================================================================================================================================================================================================================
fimage_t* nv::createNormalMap(const image_t* img, fimage_t::WrapMode wm, Vector4::Arg heightWeights, Vector4::Arg filterWeights)
{
    Kernel2* kdu = new Kernel2(9);
    kdu->initBlendedSobel(filterWeights);
    kdu->normalize();

    Kernel2* kdv = new Kernel2(*kdu);
    kdv->transpose();

    return ::createNormalMap(img, wm, heightWeights, kdu, kdv);
}


fimage_t * nv::createNormalMap(const fimage_t * img, fimage_t::WrapMode wm, Vector4::Arg filterWeights)
{
    Kernel2 * kdu = NULL;
    Kernel2 * kdv = NULL;

    kdu = new Kernel2(9);
    kdu->initBlendedSobel(filterWeights);
    kdu->normalize();

    kdv = new Kernel2(*kdu);
    kdv->transpose();

    return ::createNormalMap(img, wm, kdu, kdv);
}


//========================================================================================================================================================================================================================
// Normalize the given image in place.
//========================================================================================================================================================================================================================
void nv::normalizeNormalMap(FloatImage * img)
{
    img->normalize(0);
}

