#ifndef _fft_included_187568723451782356183745683745187436598713465897134657834
#define _fft_included_187568723451782356183745683745187436598713465897134657834

#include <cmath>

//========================================================================================================================================================================================================================
// Simple complex number class
//========================================================================================================================================================================================================================
template<typename real_t> struct complex_t
{
   real_t re, im;

   complex_t() { }
   complex_t(real_t re, real_t im) : re(re), im(im) { }

   complex_t conj() const
      { return complex_t(re, -im); }


   //====================================================================================================================================================================================================================
   // Addition
   //====================================================================================================================================================================================================================
   complex_t operator + (const complex_t& rhs) const
      { return complex_t(re + rhs.re, im + rhs.im); }

   complex_t operator + (const real_t& rhs) const
      { return complex_t(re + rhs, im); }

   complex_t& operator += (const complex_t& rhs)
   {
      re += rhs.re;
      im += rhs.im;
      return *this;
   }   

   complex_t operator += (const real_t& rhs)
   {
      re += rhs.re;
      return *this;
   } 


   //====================================================================================================================================================================================================================
   // Subtraction
   //====================================================================================================================================================================================================================
   complex_t operator - (const complex_t& rhs) const
      { return complex_t(re - rhs.re, im - rhs.im); }

   complex_t operator - (const real_t& rhs)
      { return complex_t(re - rhs, im); }

   complex_t& operator -= (const complex_t& rhs)
   {
      re -= rhs.re;
      im -= rhs.im;
      return *this;
   }   

   complex_t operator -= (const real_t& rhs)
   {
      re -= rhs.re;
      return *this;
   }


   //====================================================================================================================================================================================================================
   // Multiplication
   //====================================================================================================================================================================================================================
   complex_t operator * (const complex_t& rhs) const
      { return complex_t(re * rhs.re - im * rhs.im, re * rhs.im + im * rhs.re); }

   complex_t operator * (const real_t rhs) const
      { return complex_t(re * rhs, im * rhs); }

   complex_t operator *= (const complex_t& rhs)
   {
      real_t rr = re * rhs.re - im * rhs.im;
      im = re * rhs.im + im * rhs.re;
      re = rr;
      return *this;
   }

   complex_t operator *= (const real_t rhs)
   {
      re *= rhs;
      im *= rhs;
      return *this;
   }


   //====================================================================================================================================================================================================================
   // Division
   //====================================================================================================================================================================================================================
   complex_t operator / (const complex_t& rhs) const
   {
      real_t inv_norm = 1.0 / (rhs.re * rhs.re + rhs.im * rhs.im);
      real_t rr = re * rhs.re + im * rhs.im;
      real_t ii = im * rhs.re - re * rhs.im;
      return complex_t(rr * inv_norm, ii * inv_norm);
   }

   complex_t operator / (const real_t rhs) const
   {
      real_t inv_rhs = 1.0 / rhs;
      return complex_t(re * inv_rhs, im * inv_rhs);
   }

   complex_t operator /= (const complex_t& rhs)
   {
      real_t inv_norm = 1.0 / (rhs.re * rhs.re + rhs.im * rhs.im);
      real_t rr = re * rhs.re + im * rhs.im;
      im = (im * rhs.re - re * rhs.im) * inv_norm;
      re = rr * inv_norm;
      return *this;
   }

   complex_t operator /= (const real_t rhs)
   {
      real_t inv_rhs = 1.0 / rhs;
      re /= rhs;
      im /= rhs;
      return *this;
   }


   complex_t operator - ()
      { return complex_t(-re, -im); }

   complex_t& operator = (const complex_t& rhs)
   {
      re = rhs.re; im = rhs.im;
      return *this;
   }

   complex_t& operator = (const real_t& rhs)
   {
      re = rhs; 
      im = (real_t) 0.0;
      return *this;
   }

};

template<typename real_t> inline complex_t<real_t> iexp(real_t theta)
{
   return complex_t<real_t>(cos(theta), sin(theta));
}


namespace fft {

//========================================================================================================================================================================================================================
// 1D Fourier Transform :: brutal gothic true-metal implementation
//========================================================================================================================================================================================================================
template<typename real_t, bool inverse> void dft(complex_t<real_t>* z, unsigned int m);
template<typename real_t, bool inverse> void dft(real_t* re, real_t* im, unsigned int m);

//========================================================================================================================================================================================================================
// Fast Fourier Transform :: the main routines, one-dimensional case, complex input, strided version
//========================================================================================================================================================================================================================
template<typename real_t, bool inverse> void transform1d(complex_t<real_t>* z, unsigned int m, unsigned int stride = 1);
template<typename real_t, bool inverse> void transform1d(real_t* re, real_t* im, unsigned int m);

//========================================================================================================================================================================================================================
// 2D Fourier Transform :: the main routines
//========================================================================================================================================================================================================================
template<typename real_t, bool inverse> void transform2d(complex_t<real_t>* z, unsigned int m, unsigned int n);

} // namespace fft
#endif