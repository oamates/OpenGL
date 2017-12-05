#ifndef BOUNDEDNORMAL_H
#define BOUNDEDNORMAL_H

#include <math.h>

#include "utility.hpp"
#include "dgmath.hpp"
#include "simpleRNG.hpp"

namespace Dg
{

// Gaussian distributed random values from a predefined table.
template<typename Real> struct BoundedSND
{
    BoundedSND() : m_values(0), m_nValues(0) {}
    ~BoundedSND();

    BoundedSND(const BoundedSND&);
    BoundedSND& operator=(const BoundedSND&);

    // Initiate the table of values.
    //   a_mean Mean
    //   a_sd Standard deviation
    //   a_lower Lower bound on the normal distribution
    //   a_upper Upper bound on the normal distribution
    //   a_nValues Number of values in the table to generate.

    Dg_Result Init(Real a_mean, Real a_sd, Real a_lower, Real a_upper, unsigned int a_nValues = 1024);
    bool IsGood() const { return (m_nValues != 0); }                            // Has the object been successfully initiated?
    void Clean();                                                               // Clear the table of values.
    Real Get() const;                                                           // Get a random value from the table.
    void _init(const BoundedSND&);

    Real* m_values;
    unsigned int m_nValues;
};

//--------------------------------------------------------------------------------
//    @   BoundedSND<Real>::~BoundedSND()
//--------------------------------------------------------------------------------
template<typename Real> BoundedSND<Real>::~BoundedSND()
    { delete[] m_values; }

  //--------------------------------------------------------------------------------
  //    @   BoundedSND<Real>::_init()
  //--------------------------------------------------------------------------------
  template<typename Real>
  void BoundedSND<Real>::_init(const BoundedSND<Real>& a_other)
  {
    Clean();

    if (a_other.m_nValues)
    {
      m_nValues = a_other.m_nValues;
      m_values = (Real*)malloc(m_nValues * sizeof(Real));
      memcpy(m_values, a_other.m_values, m_nValues * sizeof(Real));
    }
  }

//--------------------------------------------------------------------------------
//    @   BoundedSND<Real>::operator=()
//--------------------------------------------------------------------------------
template<typename Real> BoundedSND<Real>::BoundedSND(const BoundedSND& a_other) : m_values(nullptr), m_nValues(0)
    { _init(a_other); }

//--------------------------------------------------------------------------------
//    @   BoundedSND<Real>::BoundedSND()
//--------------------------------------------------------------------------------
template<typename Real> BoundedSND<Real>& BoundedSND<Real>::operator = (const BoundedSND<Real>& a_other)
{
    if (this == &a_other)
        return *this;

    _init(a_other);
    return *this;
}

  //--------------------------------------------------------------------------------
  //    @   BoundedSND<Real>::Clean()
  //--------------------------------------------------------------------------------
template<typename Real> void BoundedSND<Real>::Clean()
{
    free(m_values);
    m_values = 0;
    m_nValues = 0;
}

//--------------------------------------------------------------------------------
//    @   BoundedSND<Real>::Init()
//--------------------------------------------------------------------------------
template<typename Real> Dg_Result BoundedSND<Real>::Init(Real a_mean, Real a_sd, Real a_lower, Real a_upper, unsigned int a_nValues)
{
    Clean();

    if (a_lower >= a_upper || Dg::IsZero(a_sd) || a_nValues == 0)
        return DgR_OutOfBounds;

    Real zLower = 0.5 * (1.0 + std::erf((a_lower - a_mean) / (a_sd * Dg::SQRT2)));
    Real zUpper = 0.5 * (1.0 + std::erf((a_upper - a_mean) / (a_sd * Dg::SQRT2)));

    m_nValues = a_nValues;
    m_values = new Real[m_nValues];

    for (unsigned i = 0; i < a_nValues; i++)
    {
#ifdef PRECISION_F32
        float c = static_cast<float>(zLower + (zUpper - zLower) * static_cast<float>(i) / static_cast<float>(m_nValues - 1));
        c = 2.0f * c - 1.0f;
        Real inverfResult = static_cast<Real>(inverf_f(c, 0));
#else
        double c = static_cast<double>(zLower + (zUpper - zLower) * static_cast<double>(i) / static_cast<double>(m_nValues - 1));
        c = 2.0 * c - 1.0;
        Real inverfResult = static_cast<Real>(inverf_d(c, 0));
#endif
        m_values[i] = a_sd * static_cast<Real>(SQRT2) * inverfResult + a_mean;
    }

    return DgR_Success;
}

//--------------------------------------------------------------------------------
//    @   BoundedSND<Real>::Get()
//--------------------------------------------------------------------------------
template<typename Real> Real BoundedSND<Real>::Get() const
{
    if (!m_nValues)
        return static_cast<Real>(0.0);

    SimpleRNG rng;
    unsigned int index = rng.GetUint(0, m_nValues - 1);
    return m_values[index];
}

}

#endif