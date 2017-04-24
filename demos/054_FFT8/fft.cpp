#include "fft.hpp"

#include <memory>
#include <cstring>

#include "constants.hpp"

namespace fft {

//========================================================================================================================================================================================================================
// Discrete Fourier transform :: direct implementation, complex input
//========================================================================================================================================================================================================================
template<typename real_t, bool inverse> void dft(complex_t<real_t>* z, unsigned int m)
{
    complex_t<real_t>* w = (complex_t<real_t>*) malloc(m * sizeof(complex_t<real_t>));

    for (int i = 0; i < m; ++i)
    {
        w[i] = (real_t) 0.0;
        real_t theta = -constants::two_pi_d * i / m;
        if (inverse)
            theta = -theta;
        for (int k = 0; k < m; ++k)
            w[i] += z[k] * iexp(k * theta);
    }

    std::memcpy(z, w, m * sizeof(complex_t<real_t>));
    free(w);
}

template<typename real_t, bool inverse> void dft(real_t* x, real_t* y, unsigned int m)
{
    complex_t<real_t>* w = (complex_t<real_t>*) malloc(m * sizeof(complex_t<real_t>));

    for (int i = 0; i < m; i++)
    {
        w[i] = 0.0;
        real_t theta = -constants::two_pi_d * i / m;
        if (inverse)
            theta = -theta;
        for (int k = 0; k < m; k++) 
            w[i] += complex_t<real_t>(x[k], y[k]) * iexp(k * theta);
    }

    for (int i = 0; i < m; i++)
    {
        x[i] = w[i].re;
        y[i] = w[i].im;
   }

   free(w);
}

template void dft<float,  true >(complex_t<float>*,  unsigned int);
template void dft<float,  false>(complex_t<float>*,  unsigned int);
template void dft<double, true >(complex_t<double>*, unsigned int);
template void dft<double, false>(complex_t<double>*, unsigned int);

template void dft<float,  true >(float*,  float*,  unsigned int);
template void dft<float,  false>(float*,  float*,  unsigned int);
template void dft<double, true >(double*, double*, unsigned int);
template void dft<double, false>(double*, double*, unsigned int);


//========================================================================================================================================================================================================================
// Fast Fourier Transform :: one-dimensional case, complex input, strided version
//========================================================================================================================================================================================================================
template<typename real_t, bool inverse> void transform1d(complex_t<real_t>* z, unsigned int m, unsigned int stride)
{
    unsigned int n = 1 << m;
    unsigned int i2 = n >> 1;
    unsigned int j = 0;
    for (unsigned int i = 0; i < n - 1; i++) 
    {
        if (i < j)
        {
            complex_t<real_t> t = z[stride * i];
            z[stride * i] = z[stride * j];
            z[stride * j] = t;
        }
        unsigned int k = i2;
        while (k <= j) 
        {
            j -= k;
            k >>= 1;
        }
        j += k;
    }

    complex_t<real_t> c(-1.0, 0.0); 
    unsigned int l2 = 1;
    for (unsigned int l = 0; l < m; l++)
    {
        unsigned int l1 = l2;
        l2 <<= 1;
        complex_t<real_t> u(1.0, 0.0);
        for (j = 0; j < l1; j++)
        { 
            for (unsigned int i = j; i < n; i += l2)
            {
                unsigned int i1 = i + l1;
                complex_t<real_t> t = u * z[stride * i1];
                z[stride * i1] = z[stride * i] - t;
                z[stride * i] += t;
            }
            u *= c;
        }
        c.im = -sqrt((1.0 - c.re) / 2.0);
        if (inverse)
            c.im = -c.im;
        c.re =  sqrt((1.0 + c.re) / 2.0);
    }
}


template void transform1d<float,  false>(complex_t<float>*,  unsigned int, unsigned int);
template void transform1d<float,  true> (complex_t<float>*,  unsigned int, unsigned int);
template void transform1d<double, false>(complex_t<double>*, unsigned int, unsigned int);
template void transform1d<double, true> (complex_t<double>*, unsigned int, unsigned int);

//========================================================================================================================================================================================================================
// Fast Fourier Transform :: one-dimensional case, complex input represented as two real arrays 
//========================================================================================================================================================================================================================
template<typename real_t, bool inverse> void transform1d(real_t* x, real_t* y, unsigned int m)
{
    unsigned int n = (1 << m);                                                             // The number of points
    unsigned int i2 = n >> 1;                                                              // Bit reversal
    unsigned int j = 0;

    for (unsigned int i = 0; i < n - 1; i++)
    {
        if (i < j)
        {
            real_t tx = x[i];
            real_t ty = y[i];
            x[i] = x[j];
            y[i] = y[j];
            x[j] = tx;
            y[j] = ty;
        }
        unsigned int k = i2;
        while (k <= j)
        {
            j -= k;
            k >>= 1;
        }
        j += k;
    }
   
    real_t c1 = -1.0;                                                                      // Compute the FFT
    real_t c2 =  0.0;
    unsigned int l2 = 1;
    for (unsigned int l = 0; l < m; l++)
    {
        unsigned int l1 = l2;
        l2 <<= 1;
        real_t u1 = 1.0; 
        real_t u2 = 0.0;
        for (j = 0; j < l1; j++)
        {
            for (unsigned int i = j; i < n; i += l2)
            {
                unsigned int i1 = i + l1;
                real_t t1 = u1 * x[i1] - u2 * y[i1];
                real_t t2 = u1 * y[i1] + u2 * x[i1];
                x[i1] = x[i] - t1; 
                y[i1] = y[i] - t2;
                x[i] += t1;
                y[i] += t2;
            }
            real_t z =  u1 * c1 - u2 * c2;
            u2 = u1 * c2 + u2 * c1;
            u1 = z;
        }
        c2 = -sqrt((1.0 - c1) / 2.0);
        if (inverse)
            c2 = -c2;
        c1 =  sqrt((1.0 + c1) / 2.0);
    }
}

template void transform1d<float,  false>(float*,  float*,  unsigned int);
template void transform1d<float,  true >(float*,  float*,  unsigned int);
template void transform1d<double, false>(double*, double*, unsigned int);
template void transform1d<double, true >(double*, double*, unsigned int);


//========================================================================================================================================================================================================================
// 2D Fast Fourier transform
//========================================================================================================================================================================================================================
template<typename real_t, bool inverse> void transform2d(complex_t<real_t>* z, unsigned int mx, unsigned int my)
{
   unsigned int nx = 1 << mx, ny = 1 << my;
   for (unsigned int j = 0; j < ny; j++)                                      // Transform the rows
      transform1d<real_t, inverse>(z + j * nx, mx, 1);         
   for (unsigned int i = 0; i < nx; i++)                                      // Transform the columns
      transform1d<real_t, inverse>(z + i, my, nx);
}

template void transform2d<float,  false>(complex_t<float>*,  unsigned int, unsigned int);
template void transform2d<float,  true> (complex_t<float>*,  unsigned int, unsigned int);
template void transform2d<double, false>(complex_t<double>*, unsigned int, unsigned int);
template void transform2d<double, true> (complex_t<double>*, unsigned int, unsigned int);

}