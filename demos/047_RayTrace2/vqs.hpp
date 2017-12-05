#ifndef VQS_H
#define VQS_H

#include "vector4.hpp"
#include "quaternion.hpp"
#include "matrix44.hpp"
#include "dgmath.hpp"

namespace Dg
{

template<typename Real> struct VQS;

template<typename Real> VQS<Real> Inverse(VQS<Real> const &);

// struct VQS : Vector, quaternion, scalar structure
// This is an alternate to matrices to represent transformations in 3D space. In this class we have a translation component
// (a vector), orientation (a quaternion) and scaling component (scalar). The main advantage over a 4x4 matrix is using a quaternion 
// for orientation, arguably a more complete and robust method for orientation representation. 
//
// Some disadvantages include: we are restricted to the types of transformations
// we can do. We cannot shear or reflect for example. Also We must use uniform scaling.

template<typename Real> struct VQS
{
    Vector4<Real> m_v;                                                      // translation
    Quaternion<Real> m_q;                                                   // rotation
    Real m_s;                                                               // scale

    VQS() : m_q(static_cast<Real>(1.0), static_cast<Real>(0.0), static_cast<Real>(0.0), static_cast<Real>(0.0)), 
            m_s(static_cast<Real>(1.0)) 
        { m_v.Zero(); }

    VQS(Vector4<Real> const & a_v, Quaternion<Real> const & a_q, Real a_s) : m_v(a_v), m_q(a_q), m_s(a_s) {}
    ~VQS() {}

    VQS(const VQS<Real>& a_other) : m_v(a_other.m_v), m_q(a_other.m_q), m_s(a_other.m_s) {}
    VQS<Real>& operator = (const VQS<Real>&);

    bool operator == (const VQS<Real>& a_other) const;                      // Comparison
    bool operator != (const VQS<Real>& a_other) const;
    void MakeValid();                                                       // Ensure a valid VQS
    void Identity();                                                        // Make identity
    void Set(const Matrix44<Real>&);                                        // Set VQS based on an affine matrix
    void Set(const Vector4<Real>&, const Quaternion<Real>&, Real);
    void SetV(const Vector4<Real>&);
    void SetQ(const Quaternion<Real>&);
    void SetS(Real);
    void UpdateV(const Vector4<Real>&);                                     // Translation update data
    void UpdateQ(const Quaternion<Real>&);                                  // Quaternion update data
    void UpdateS(Real);                                                     // Scalar update data
    VQS<Real> operator * (const VQS<Real>&) const;                          // VQS structures concatenate left to right.
    VQS<Real>& operator *= (const VQS<Real>&);
    Vector4<Real> TransformPoint(const Vector4<Real>&);                     // Point transformations also apply translation.
    Vector4<Real> TransformVector(const Vector4<Real>&);                    // Vector transformations do not apply translation.
    Vector4<Real>& TransformPointSelf(Vector4<Real>&);                      // Point transformations also apply translation.
    Vector4<Real>& TransformVectorSelf(Vector4<Real>&);                     // Vector transformations do not apply translation.
    Vector4<Real> Translate(const Vector4<Real>&) const;                    // Apply translation to Vector4.
    Vector4<Real> Rotate(const Vector4<Real>&) const;                       // Apply rotation to Vector4.
    Vector4<Real> Scale(const Vector4<Real>&) const;                        // Apply scale to Vector4.
    void TranslateSelf(Vector4<Real>&) const;                               // Apply translation to Vector4.
    void RotateSelf(Vector4<Real>&) const;                                  // Apply rotation to Vector4.
    void ScaleSelf(Vector4<Real>&) const;                                   // Apply scale to Vector4.
    const VQS& Inverse();                                                   // Inverse.
    template<typename T> friend VQS<T> Inverse(const VQS<T>&);              // Inverse.
    void Get(Vector4<Real>& a_v, Quaternion<Real>& a_q, Real& a_s) const;
    void GetMatrix(Matrix44<Real>&) const;                                  // Conversion to Matrix.

    const Vector4<Real>& V() const
        { return m_v; }
    const Quaternion<Real>& Q() const
        { return m_q; }
    Real S() const
        { return m_s; }

};

//--------------------------------------------------------------------------------
//    @   VQS<Real>::Set()
//--------------------------------------------------------------------------------
template<typename Real> void VQS<Real>::Set(Matrix44<Real> const & a_m)
{
    m_v.m_x = a_m[12];
    m_v.m_y = a_m[13];
    m_v.m_z = a_m[14];
    a_m.GetQuaternion(m_q);
    m_s = sqrt((a_m[0] * a_m[0]) + (a_m[4] * a_m[4]) + (a_m[8] * a_m[8]));  // Just use matrix x-scaling
}

//--------------------------------------------------------------------------------
//    @   VQS<Real>::Set()
//--------------------------------------------------------------------------------
template<typename Real> void VQS<Real>::SetV(Vector4<Real> const & a_v)
{
    m_v = a_v;
    m_v.m_V[3] = static_cast<Real>(0.0);
}

//--------------------------------------------------------------------------------
//    @   VQS<Real>::SetQ()
//--------------------------------------------------------------------------------
template<typename Real> void VQS<Real>::SetQ(Quaternion<Real> const & a_q)
{
    m_q = a_q;
    m_q.MakeValid();
}

//--------------------------------------------------------------------------------
//    @   VQS<Real>::SetS()
//--------------------------------------------------------------------------------
template<typename Real> void VQS<Real>::SetS(Real a_s)
    { m_s = (a_s < Dg::EPSILON) ? static_cast<Real> (0.0) : a_s; }

//--------------------------------------------------------------------------------
// VQS<Real>::UpdateV() -- Update translation vector
//--------------------------------------------------------------------------------
template<typename Real> void VQS<Real>::UpdateV(Vector4<Real> const & a_v)
{
    m_v += a_v;
    m_v.m_w = static_cast<Real>(0.0);
}

//--------------------------------------------------------------------------------
//    @   VQS<Real>::UpdateQ() -- Update quaternion
//--------------------------------------------------------------------------------
template<typename Real> void VQS<Real>::UpdateQ(Quaternion<Real> const & a_q)
    { m_q *= a_q; }

//--------------------------------------------------------------------------------
//    @   VQS<Real>::UpdateS()
//--------------------------------------------------------------------------------
template<typename Real> void VQS<Real>::UpdateS(Real a_s)
    { m_s *= a_s; }

//--------------------------------------------------------------------------------
//    @   VQS<Real>::operator==()
//--------------------------------------------------------------------------------
template<typename Real> bool VQS<Real>::operator == (const VQS<Real>& a_other) const
    { return !((a_other.m_v != m_v) || (a_other.m_q != m_q) || (a_other.m_s != m_s)); }

//--------------------------------------------------------------------------------
//    @   VQS<Real>::operator!=()
//--------------------------------------------------------------------------------
template<typename Real> bool VQS<Real>::operator != (const VQS<Real>& a_other) const
    { return ((a_other.m_v != m_v) || (a_other.m_q != m_q) || (a_other.m_s != m_s)); }

//--------------------------------------------------------------------------------
//    @   VQS<Real>::operator=()
//--------------------------------------------------------------------------------
template<typename Real> VQS<Real>& VQS<Real>::operator = (const VQS<Real>& a_other)
{
    m_v = a_other.m_v;
    m_q = a_other.m_q;
    m_s = a_other.m_s;
    return *this;
}

//--------------------------------------------------------------------------------
//    @   VQS<Real>::MakeValid
//--------------------------------------------------------------------------------
template<typename Real> void VQS<Real>::MakeValid()
{
    m_v.m_V[3] = static_cast<Real>(0.0);            // Clean vector
    m_q.MakeValid();                                // Clean quaternion
    if (m_s < Dg::EPSILON)                          // Clean scale;
        m_s = static_cast<Real>(0.0);
}

//--------------------------------------------------------------------------------
//    @   VQS<Real>::Identity()
//--------------------------------------------------------------------------------
template<typename Real> void VQS<Real>::Identity()
{
    m_v.Zero();
    m_q.Identity();
    m_s = static_cast<Real>(1.0);
}

//--------------------------------------------------------------------------------
//    @   TransformPoint()
//--------------------------------------------------------------------------------
template<typename Real> Vector4<Real> VQS<Real>::TransformPoint(const Vector4<Real>& a_v)
{
    Vector4<Real> result(a_v);
    result.m_x *= m_s;                              // scale
    result.m_y *= m_s;
    result.m_z *= m_s;
    m_q.RotateSelf(result);                         // rotate
    result += m_v;                                  // translate
    return result;
}

//--------------------------------------------------------------------------------
//    @   TransformPointSelf()
//--------------------------------------------------------------------------------
template<typename Real> Vector4<Real>& VQS<Real>::TransformPointSelf(Vector4<Real>& a_v)
{
    a_v.m_x *= m_s;                                 // scale
    a_v.m_y *= m_s;
    a_v.m_z *= m_s;
    m_q.RotateSelf(a_v);                            // rotate
    a_v += m_v;                                     // translate
    return a_v;
}

//--------------------------------------------------------------------------------
//    @   TransformVector()
//--------------------------------------------------------------------------------
template<typename Real> Vector4<Real> VQS<Real>::TransformVector(const Vector4<Real>& a_v)
{
    Vector4<Real> result(a_v);
    result.m_V[0] *= m_s;                           // scale
    result.m_V[1] *= m_s;
    result.m_V[2] *= m_s;
    m_q.RotateSelf(result);                         // rotate
    return result;
}

//--------------------------------------------------------------------------------
//    @   TransformVectorSelf()
//--------------------------------------------------------------------------------
template<typename Real> Vector4<Real>& VQS<Real>::TransformVectorSelf(Vector4<Real>& a_v)
{
    a_v.m_x *= m_s;                                 // scale
    a_v.m_y *= m_s;
    a_v.m_z *= m_s;
    m_q.RotateSelf(a_v);                            // rotate
    return a_v;
}

//--------------------------------------------------------------------------------
//    @   VQS<Real>::Translate()
//--------------------------------------------------------------------------------
template<typename Real> Vector4<Real> VQS<Real>::Translate(const Vector4<Real>& a_v) const
{
    Vector4<Real> result(a_v);
    result += m_v;
    return result;
}

//--------------------------------------------------------------------------------
//    @   VQS<Real>::Rotate()
//--------------------------------------------------------------------------------
template<typename Real> Vector4<Real> VQS<Real>::Rotate(Vector4<Real> const & a_v) const
{
    Vector4<Real> result(a_v);
    m_q.RotateSelf(result);
    return result;
}

//--------------------------------------------------------------------------------
//    @   VQS<Real>::Scale()
//--------------------------------------------------------------------------------
template<typename Real> Vector4<Real> VQS<Real>::Scale(Vector4<Real> const & a_v) const
{
    Vector4<Real> result(a_v);
    result.m_x *= m_s;
    result.m_y *= m_s;
    result.m_z *= m_s;
    return result;
}

//--------------------------------------------------------------------------------
//    @   VQS<Real>::TranslateSelf()
//--------------------------------------------------------------------------------
template<typename Real> void VQS<Real>::TranslateSelf(Vector4<Real>& a_v) const
{
    a_v.x += m_v.x;
    a_v.y += m_v.y;
    a_v.z += m_v.z;
}

//--------------------------------------------------------------------------------
//    @   VQS<Real>::RotateSelf()
//--------------------------------------------------------------------------------
template<typename Real> void VQS<Real>::RotateSelf(Vector4<Real>& a_v) const
    { m_q.RotateSelf(a_v); }

//--------------------------------------------------------------------------------
//    @   VQS<Real>::ScaleSelf()
//--------------------------------------------------------------------------------
template<typename Real> void VQS<Real>::ScaleSelf(Vector4<Real>& a_v) const
{
    a_v.x *= m_s;
    a_v.y *= m_s;
    a_v.z *= m_s;
}

//--------------------------------------------------------------------------------
//    @   VQS<Real>::Set()
//--------------------------------------------------------------------------------
template<typename Real> void VQS<Real>::Set(const Vector4<Real>& a_v, const Quaternion<Real>& a_q, Real a_s)
{
    m_v = a_v;
    m_q = a_q;
    m_s = a_s;
    MakeValid();
}

//--------------------------------------------------------------------------------
//    @   VQS<Real>::Get()
//--------------------------------------------------------------------------------
template<typename Real> void VQS<Real>::Get(Vector4<Real>& a_v, Quaternion<Real>& a_q, Real& a_s) const
{
    a_v = m_v;
    a_q = m_q;
    a_s = m_s;
}

//--------------------------------------------------------------------------------
//    @   VQS<Real>::Get()
//--------------------------------------------------------------------------------
template<typename Real> void VQS<Real>::GetMatrix(Matrix44<Real>& a_out) const
{
    a_out.Rotation(m_q);
    a_out.m_V[0] *= m_s;
    a_out.m_V[1] *= m_s;
    a_out.m_V[2] *= m_s;
    a_out.m_V[4] *= m_s;
    a_out.m_V[5] *= m_s;
    a_out.m_V[6] *= m_s;
    a_out.m_V[8] *= m_s;
    a_out.m_V[9] *= m_s;
    a_out.m_V[10] *= m_s;
    a_out.m_V[12] = m_v.m_x *= m_s;
    a_out.m_V[13] = m_v.m_y *= m_s;
    a_out.m_V[14] = m_v.m_z *= m_s;
}

//--------------------------------------------------------------------------------
//    @   VQS<Real>::operator*()
//--------------------------------------------------------------------------------
template<typename Real> VQS<Real> VQS<Real>::operator*(const VQS<Real>& a_rhs) const
{
    VQS<Real> result(*this);
    result.m_v += (m_q.Rotate(a_rhs.m_v) * m_s);                // Combine translation vectors
    result.m_q *= a_rhs.m_q;                                    // Combine quaternions
    result.m_s *= a_rhs.m_s;                                    // Combine scales
    return result;
}

//--------------------------------------------------------------------------------
//    @   VQS<Real>::Operator*=()
//--------------------------------------------------------------------------------
template<typename Real> VQS<Real>& VQS<Real>::operator *= (const VQS<Real>& a_rhs)
{
    m_v += (m_q.Rotate(a_rhs.m_v) * m_s);                       // Combine translation vectors
    m_q *= a_rhs.m_q;                                           // Combine quaternions
    m_s *= a_rhs.m_s;                                           // Combine scales
    return *this;
}


//--------------------------------------------------------------------------------
// VQS inverse: [1 / m_s * (m_q - 1 * (-m_v) * m_q), m_q - 1, 1 / m_s]
//--------------------------------------------------------------------------------
template<typename Real> VQS<Real> const & VQS<Real>::Inverse()
{
    m_s = static_cast<Real>(1.0) / m_s;                         // Inverse scale
    m_q.Inverse();                                              // Inverse quaternion
    m_v = m_q.Rotate(-m_v) * m_s;                               // Inverse vector
    return *this;
}

//--------------------------------------------------------------------------------
//    @   Inverse()
//--------------------------------------------------------------------------------
template<typename Real> VQS<Real> Inverse(VQS<Real> const & a_other)
{
    VQS<Real> temp;
    temp.m_s = static_cast<Real>(1.0) / a_other.m_s;            // Inverse scale
    temp.m_q = Inverse(a_other.m_q);                            // Inverse quaternion
    temp.m_v = temp.m_q.Rotate(-a_other.m_v) * temp.m_s;        // Inverse vector
    return temp;
}

}

#endif