#ifndef QUATERNION_H
#define QUATERNION_H

#include "dgmath.hpp"
#include "vector4.hpp"

namespace Dg
{

template<typename Real> class Vector4;
template<typename Real> class Quaternion;
template<typename Real> class VQS;

template<typename Real> Quaternion<Real> operator*(Real, const Quaternion<Real>&);
template<typename Real> Real Dot(const Quaternion<Real>&, const Quaternion<Real>&);
template<typename Real> Quaternion<Real> Conjugate(const Quaternion<Real>&);
template<typename Real> Quaternion<Real> Inverse(const Quaternion<Real>&);
template<typename Real> void Lerp(Quaternion<Real>& a_result, const Quaternion<Real>& a_start, const Quaternion<Real>& a_end, Real a_t);
template<typename Real> void Slerp(Quaternion<Real>& a_result, const Quaternion<Real>& a_start, const Quaternion<Real>& a_end, Real a_t);
template<typename Real> void ApproxSlerp(Quaternion<Real>& a_result, const Quaternion<Real>& a_start, const Quaternion<Real>& a_end, Real a_t);
template<typename Real> Quaternion<Real> Conjugate(const Quaternion<Real>&);

// Quaternion
// Three dimensional homogeneous vector class [x, y, z, w].
//
// Quaternion class with methods tailored for rotating points and vectors. The quaternion 4-tuple can theoretically take on any values, 
// however for orientation representation in 3D space, it is required the quaternion is normalised. It is up to the user to ensure the 
// quaternion is normalised if setting each element itself, for eg
//
//       Quaternion q(1.0, 2.3, -2.9, 4.6);  //Not a valid rotational quaternion
//       q.Normalize();                      //Valid rotational quaternion
//
// Quaternions constructed or set by any other means will be valid for eg
//
//       void Set( Real zRotation, Real yRotation, Real xRotation );
//
// Quaternion concatenation uses left-to-right convention. For example the follow lines are equivalent
//
//     q_final = q_0 * q_1 * q_2 ... * q_n;
//     q_final = ( ... ((q_0 * q_1) * q_2) ... * q_n);
//

template<typename Real> struct Quaternion
{
    Real m_w, m_x, m_y, m_z;
    
    // Default constructed to the identity quaternion (w(1), x(0), y(0), z(0)).
    Quaternion() : m_w(static_cast<Real>(1.0)), m_x(static_cast<Real>(0.0)), m_y(static_cast<Real>(0.0)), m_z(static_cast<Real>(0.0)) {}
    Quaternion(Real a_w, Real a_x, Real a_y, Real a_z) :
      m_w(a_w), m_x(a_x), m_y(a_y), m_z(a_z) {}
    ~Quaternion() {}

    
    Quaternion(const Vector4<Real>& axis, Real angle);                  // Construct quaternion based on axis-angle.
    Quaternion(const Vector4<Real>& from, const Vector4<Real>& to);     // Construct quaternion based on start and end vectors.
    explicit Quaternion(const Vector4<Real>&);                          // Construct quaternion from vector elements.
    Quaternion(const Quaternion&);
    Quaternion& operator=(const Quaternion&);

    Real& operator [] (unsigned int a_i)                                // Accessor member by index
        { return (&m_w)[a_i]; }
    
    Real operator [] (unsigned int a_i) const                           // Accessor member by index
        { return (&m_w)[a_i]; }
    
    Real Magnitude() const;                                             // Length.
    Real Norm() const;                                                  // Squared length.
    
    bool operator == (const Quaternion&) const;                         // comparison
    bool operator != (const Quaternion&) const;
    
    bool IsZero() const;                                                // Determines if the quaternion is the zero quaternion within some tolerance.
    bool IsUnit() const;                                                // Determines if the quaternion is the unit quaternion within some tolerance.
    bool IsIdentity() const;                                            // Determines if the quaternion is the identity quaternion within some tolerance.

    void Set(Real a_w, Real a_x, Real a_y, Real a_z);                   // Set quaternion elements.
    void SetRotationX(Real);                                            // Set quaternion based on rotation about the x axis.
    void SetRotationY(Real);                                            // Set quaternion based on rotation about the y axis.
    void SetRotationZ(Real);                                            // Set quaternion based on rotation about the z axis.
    void SetRotation(Real rx, Real ry, Real rz, EulerOrder);            // Set rotation quaternion.
    void Set(const Vector4<Real>& axis, Real angle);                    // Set quaternion based on axis-angle.
    void Set(const Vector4<Real>& from, const Vector4<Real>& to);       // Set quaternion based on start and end vectors.
    void MakeValid();                                                   // Ensures quaternion is a valid rotational quaternion.
    void GetAxisAngle(Vector4<Real>& axis, Real& angle);                // Get axis-angle based on quaternion.

    // Get the set of basis vectors associated with the quaternion
    void GetBasis(Vector4<Real>& a_x0, Vector4<Real>& a_x1, Vector4<Real>& a_x2) const;

    void Clean();                                                       // Sets near-zero elements to 0.
    inline void Zero();                                                 // Sets all elements to 0.
    void Normalize();                                                   // Sets to unit quaternion.
    void Identity();                                                    // Sets to identity quaternion.

    // Compute complex conjugate.
    template<typename T> friend Quaternion<T> Conjugate(const Quaternion<T>&);
    const Quaternion& Conjugate();                                      // Set self to complex conjugate.

    // Invert quaternion.
    template<typename T> friend Quaternion<T> Inverse(const Quaternion<T>&);
    const Quaternion& Inverse();                                        // Set self to inverse.

    Quaternion operator + (const Quaternion&) const;
    Quaternion& operator += (const Quaternion&);
    Quaternion operator - (const Quaternion&) const;
    Quaternion& operator -= (const Quaternion&);
    Quaternion operator - () const;

    // scalar multiplication
    template<typename T> friend Quaternion<T> operator*(T scalar, const Quaternion<T>& vector);

    Quaternion& operator *= (Real);
    Quaternion operator * (const Quaternion&) const;                    // Quaternion concatenation.
    Quaternion& operator *= (const Quaternion&);                        // Quaternion concatenation.

    template<typename T> friend T Dot(const Quaternion<T>&, const Quaternion<T>&);

    Vector4<Real> Rotate(const Vector4<Real>&) const;                   // Vector rotation. Quaternion is normalized.
    void RotateSelf(Vector4<Real>&) const;                              // Vector rotation. Quaternion is normalized.

    // Linearly interpolate two quaternions. This will always take the shorter path between them.
    template<typename T> friend void Lerp(Quaternion<T>& a_result, const Quaternion<T>& a_start, const Quaternion<T>& a_end, T a_t);
    
    // Spherical linearly interpolate two quaternions. This will always take the shorter path between them.
    template<typename T> friend void Slerp(Quaternion<T>& a_result, const Quaternion<T>& a_start, const Quaternion<T>& a_end, T a_t);

    // Approximate spherical linear interpolation of two quaternions.
    template<typename T> friend void ApproxSlerp(Quaternion<T>& a_result, const Quaternion<T>& a_start, const Quaternion<T>& a_end, T a_t);
};


//-------------------------------------------------------------------------------
//    @   Quaternion::Set()
//-------------------------------------------------------------------------------
template<typename Real> void Quaternion<Real>::Set(Real a_w, Real a_x, Real a_y, Real a_z)
{
    m_w = a_w; 
    m_x = a_x; 
    m_y = a_y; 
    m_z = a_z;
}

//-------------------------------------------------------------------------------
//    @   Quaternion::Zero()
//-------------------------------------------------------------------------------
template<typename Real> void Quaternion<Real>::Zero()
    { m_x = m_y = m_z = m_w = static_cast<Real>(0.0); }

//-------------------------------------------------------------------------------
//    @   Quaternion::Identity()
//-------------------------------------------------------------------------------
template<typename Real> void Quaternion<Real>::Identity()
{
    m_x = m_y = m_z = static_cast<Real>(0.0);
    m_w = static_cast<Real>(1.0);
}

//-------------------------------------------------------------------------------
//    @   Quaternion::Quaternion()
//-------------------------------------------------------------------------------
template<typename Real> Quaternion<Real>::Quaternion(const Vector4<Real>& a_axis, Real a_angle)
    { Set(a_axis, a_angle); }

//-------------------------------------------------------------------------------
//    @   Quaternion::Quaternion()
//-------------------------------------------------------------------------------
template<typename Real> Quaternion<Real>::Quaternion(const Vector4<Real>& a_from, const Vector4<Real>& a_to)
    { Set(a_from, a_to); }

//-------------------------------------------------------------------------------
//    @   Quaternion::Quaternion()
//-------------------------------------------------------------------------------
template<typename Real> Quaternion<Real>::Quaternion(const Vector4<Real>& a_vector)
    { Set(static_cast<Real>(0.0), a_vector.m_V[0], a_vector.m_V[1], a_vector.m_V[2]); }

//-------------------------------------------------------------------------------
//    @   Quaternion::Quaternion()
//-------------------------------------------------------------------------------
template<typename Real> Quaternion<Real>::Quaternion(const Quaternion<Real>& a_other)
    : m_w(a_other.m_w), m_x(a_other.m_x), m_y(a_other.m_y), m_z(a_other.m_z)
    { }

//-------------------------------------------------------------------------------
//    @   Quaternion::operator=()
//-------------------------------------------------------------------------------
template<typename Real> Quaternion<Real>& Quaternion<Real>::operator=(const Quaternion<Real>& a_other)
{
    m_w = a_other.m_w;
    m_x = a_other.m_x;
    m_y = a_other.m_y;
    m_z = a_other.m_z;
    return *this;
}

//-------------------------------------------------------------------------------
//    @   Quaternion::Magnitude()
//-------------------------------------------------------------------------------
template<typename Real> Real Quaternion<Real>::Magnitude() const
    { return sqrt(m_w * m_w + m_x * m_x + m_y * m_y + m_z * m_z); }

//-------------------------------------------------------------------------------
//    @   Quaternion::Norm()
//-------------------------------------------------------------------------------
template<typename Real> Real Quaternion<Real>::Norm() const
    { return (m_w*m_w + m_x*m_x + m_y*m_y + m_z*m_z); }

//-------------------------------------------------------------------------------
//    @   Quaternion::operator==()
//-------------------------------------------------------------------------------
template<typename Real> bool Quaternion<Real>::operator == (const Quaternion<Real>& a_other) const
{
    return Dg::IsZero(a_other.m_w - m_w) && Dg::IsZero(a_other.m_x - m_x) &&
           Dg::IsZero(a_other.m_y - m_y) && Dg::IsZero(a_other.m_z - m_z);
}

//-------------------------------------------------------------------------------
//    @   Quaternion::operator!=()
//-------------------------------------------------------------------------------
template<typename Real> bool Quaternion<Real>::operator != (const Quaternion<Real>& a_other) const
{
   return !(Dg::IsZero(a_other.m_w - m_w) && Dg::IsZero(a_other.m_x - m_x) &&
            Dg::IsZero(a_other.m_y - m_y) && Dg::IsZero(a_other.m_z - m_z));
}

//-------------------------------------------------------------------------------
//    @   Quaternion::IsZero()
//-------------------------------------------------------------------------------
template<typename Real> bool Quaternion<Real>::IsZero() const
    { return Dg::IsZero(m_w * m_w + m_x * m_x + m_y * m_y + m_z * m_z); }

//-------------------------------------------------------------------------------
//    @   Quaternion::IsUnit()
//-------------------------------------------------------------------------------
template<typename Real> bool Quaternion<Real>::IsUnit() const
    { return Dg::IsZero(static_cast<Real>(1.0) - m_w * m_w - m_x * m_x - m_y * m_y - m_z * m_z); }

//-------------------------------------------------------------------------------
//    @   Quaternion::IsIdentity()
//-------------------------------------------------------------------------------
template<typename Real> bool Quaternion<Real>::IsIdentity() const
{
    return (Dg::IsZero(static_cast<Real>(1.0) - m_w) && 
            Dg::IsZero(m_x) && Dg::IsZero(m_y) && Dg::IsZero(m_z));
}

//-------------------------------------------------------------------------------
//    @   Quaternion::SetRotationX()
//-------------------------------------------------------------------------------
template<typename Real> void Quaternion<Real>::SetRotationX(Real a_rx)
{
    a_rx *= static_cast<Real>(0.5);
    m_w = static_cast<Real>(cos(a_rx));
    m_x = static_cast<Real>(sin(a_rx));
    m_y = static_cast<Real>(0.0);
    m_z = m_y;
}

//-------------------------------------------------------------------------------
//    @   Quaternion::SetRotationZ()
//-------------------------------------------------------------------------------
template<typename Real> void Quaternion<Real>::SetRotationY(Real a_ry)
{
    a_ry *= static_cast<Real>(0.5);
    m_w = static_cast<Real>(cos(a_ry));
    m_x = static_cast<Real>(0.0);
    m_y = static_cast<Real>(sin(a_ry));
    m_z = m_x;
}

//-------------------------------------------------------------------------------
//    @   Quaternion::SetRotation()
//-------------------------------------------------------------------------------
template<typename Real> void Quaternion<Real>::SetRotationZ(Real a_rz)
{
    a_rz *= static_cast<Real>(0.5);
    m_w = static_cast<Real>(cos(a_rz));
    m_x = static_cast<Real>(0.0);
    m_y = m_x;
    m_z = static_cast<Real>(sin(a_rz));
}

//-------------------------------------------------------------------------------
//    @   Quaternion::SetRotation()
//-------------------------------------------------------------------------------
template<typename Real> void Quaternion<Real>::SetRotation(Real a_rx, Real a_ry, Real a_rz, EulerOrder a_order)
{
    a_rx *= static_cast<Real>(0.5);
    a_ry *= static_cast<Real>(0.5);
    a_rz *= static_cast<Real>(0.5);

    
    Real Cx = Real(cos(a_rx)); // sines and cosines of half angles
    Real Sx = Real(sin(a_rx));
    Real Cy = Real(cos(a_ry));
    Real Sy = Real(sin(a_ry));
    Real Cz = Real(cos(a_rz));
    Real Sz = Real(sin(a_rz));

    switch (a_order)
    {
        case EulerOrder::ZYX:
            m_w = Cx * Cy * Cz - Sx * Sy * Sz;
            m_x = Sx * Cy * Cz + Cx * Sy * Sz;
            m_y = Cx * Sy * Cz - Sx * Cy * Sz;
            m_z = Cx * Cy * Sz + Sx * Sy * Cz;
        break;
        case EulerOrder::YZX:
            m_w = Cx * Cy * Cz + Sx * Sy * Sz;
            m_x = Sx * Cy * Cz - Cx * Sy * Sz;
            m_y = Cx * Sy * Cz - Sx * Cy * Sz;
            m_z = Cx * Cy * Sz + Sx * Sy * Cz;
        break;
        case EulerOrder::ZXY:
            m_w = Cx * Cy * Cz + Sx * Sy * Sz;
            m_x = Sx * Cy * Cz + Cx * Sy * Sz;
            m_y = Cx * Sy * Cz - Sx * Cy * Sz;
            m_z = Cx * Cy * Sz - Sx * Sy * Cz;
        break;
        case EulerOrder::XZY:
            m_w = Cx * Cy * Cz - Sx * Sy * Sz;
            m_x = Sx * Cy * Cz + Cx * Sy * Sz;
            m_y = Cx * Sy * Cz + Sx * Cy * Sz;
            m_z = Cx * Cy * Sz - Sx * Sy * Cz;
        break;
        case EulerOrder::XYZ:
            m_w = Cx * Cy * Cz + Sx * Sy * Sz;
            m_x = Sx * Cy * Cz - Cx * Sy * Sz;
            m_y = Cx * Sy * Cz + Sx * Cy * Sz;
            m_z = Cx * Cy * Sz - Sx * Sy * Cz;
        break;
        case EulerOrder::YXZ:
            m_w = Cx * Cy * Cz - Sx * Sy * Sz;
            m_x = Sx * Cy * Cz - Cx * Sy * Sz;
            m_y = Cx * Sy * Cz + Sx * Cy * Sz;
            m_z = Cx * Cy * Sz + Sx * Sy * Cz;
        break;
        case EulerOrder::XYX:
            m_w = Cz * Cy * Cx - Sz * Cy * Sx;
            m_x = Cz * Cy * Sx + Sz * Cy * Cx;
            m_y = Cz * Sy * Cx + Sz * Sy * Sx;
            m_z = Sz * Sy * Cx - Cz * Sy * Sx;
        break;
        case EulerOrder::XZX:
            m_w = Cz * Cy * Cx - Sz * Cy * Sx;
            m_x = Cz * Cy * Sx + Sz * Cy * Cx;
            m_y = Cz * Sy * Sx - Sz * Sy * Cx;
            m_z = Cz * Sy * Cx + Sz * Sy * Sx;
        break;
        case EulerOrder::YXY:
            m_w = Cz * Cy * Cx - Sz * Cy * Sx;
            m_x = Sz * Sy * Sx + Cz * Sy * Cx;
            m_y = Cz * Cy * Sx + Sz * Cy * Cx;
            m_z = Cz * Sy * Sx - Sz * Sy * Cx;
        break;
        case EulerOrder::YZY:
            m_w = Cz * Cy * Cx - Sz * Cy * Sx;
            m_x = Sz * Sy * Cx - Cz * Sy * Sx;
            m_y = Cz * Cy * Sx + Sz * Cy * Cx;
            m_z = Cz * Sy * Cx + Sz * Sy * Sx;
        break;
        case EulerOrder::ZXZ:
            m_w = Cz * Cy * Cx - Sz * Cy * Sx;
            m_x = Cz * Sy * Cx + Sz * Sy * Sx;
            m_y = Sz * Sy * Cx - Cz * Sy * Sx;
            m_z = Cz * Cy * Sx + Sz * Cy * Cx;
        break;
        case EulerOrder::ZYZ:
            m_w = Cz * Cy * Cx - Sz * Cy * Sx;
            m_x = Cz * Sy * Sx - Sz * Sy * Cx;
            m_y = Cz * Sy * Cx + Sz * Sy * Sx;
            m_z = Cz * Cy * Sx + Sz * Cy * Cx;
        break;
    }
}

//-------------------------------------------------------------------------------
//    @   Quaternion::Set()
//-------------------------------------------------------------------------------
template<typename Real> void Quaternion<Real>::Set(const Vector4<Real>& a_axis, Real a_angle)
{
    Real length = a_axis.LengthSquared();                               // if axis of rotation is zero vector, just set to identity quat
    if (Dg::IsZero(length))
    {
        Identity();
        return;
    }
    a_angle *= static_cast<Real>(0.5);                                  // take half-angle
    Real sintheta = Real(sin(a_angle));
    Real costheta = Real(cos(a_angle));
    Real scaleFactor = sintheta / Real(sqrt(length));
    m_w = costheta;
    m_x = scaleFactor * a_axis.m_V[0];
    m_y = scaleFactor * a_axis.m_V[1];
    m_z = scaleFactor * a_axis.m_V[2];
}

//-------------------------------------------------------------------------------
//    @   Quaternion::()
//-------------------------------------------------------------------------------
template<typename Real> void Quaternion<Real>::Set(const Vector4<Real>& a_from, const Vector4<Real>& a_to)
{
    Vector4<Real> axis = Cross<Real>(a_from, a_to);                 // get axis of rotation
    Set(Dot(a_from, a_to), axis.m_V[0], axis.m_V[1], axis.m_V[2]);  // get scaled cos of angle between vectors and set initial quaternion

    // quaternion at this point is ||from||*||a_to||*( cos(theta), r*sin(theta) )
    // normalize a_to remove ||from||*||a_to|| factor
    Normalize();

    // quaternion at this point is (cos(theta), r*sin(theta))
    // what we want is ( cos(theta/2), r*sin(theta/2) )
    // set up for half angle calculation
    m_w += static_cast<Real>(1.0f);

    // now when we normalize, we'll be dividing by sqrt(2*(1 + cos(theta))), which is 
    // what we want for r * sin(theta) a_to give us r * sin(theta / 2)
    // before we normalize, check if vectors are opposing
    if (m_w <= static_cast<Real>(EPSILON_d))                        
    {
        // rotate pi radians around orthogonal vector take cross product with x axis        
        if (a_from.m_V[2]*a_from.m_V[2] > a_from.m_V[0]*a_from.m_V[0])
            Set(static_cast<Real>(0.0), static_cast<Real>(0.0), a_from.m_V[2], -a_from.m_V[1]);
        else                                                        // or take cross product with z axis
            Set(static_cast<Real>(0.0), a_from.m_V[1], -a_from.m_V[0], static_cast<Real>(0.0));
    }
    Normalize();                                                    // normalize again a_to get rotation quaternion
}

//-------------------------------------------------------------------------------
//    @   Quaternion::MakeValid()
//-------------------------------------------------------------------------------
template<typename Real> void Quaternion<Real>::MakeValid()
{
    Clean();                                                        // Set near-zero elements to zero
    Real lengthsq = m_w * m_w + m_x * m_x + m_y * m_y + m_z * m_z;  // Find Norm
    if (Dg::IsZero(static_cast<Real>(1.0) - lengthsq))              // Return if valid
        return;
    if (Dg::IsZero(lengthsq))                                       // If zero quaternion
    {
        m_w = static_cast<Real>(1.0);
        m_x = m_y = m_z = static_cast<Real>(0.0);
    }
    else
    {
        Real factor = static_cast<Real>(1.0) / Real(sqrt(lengthsq));
        m_w *= factor;
        m_x *= factor;
        m_y *= factor;
        m_z *= factor;
    }
}

//-------------------------------------------------------------------------------
//    @   Quaternion::GetAxisAngle()
//-------------------------------------------------------------------------------
template<typename Real> void Quaternion<Real>::GetAxisAngle(Vector4<Real>& a_axis, Real& a_angle)
{
    a_angle = static_cast<Real>(2.0) * Real(acos(m_w));
    Real length = sqrt(static_cast<Real>(1.0) - m_w * m_w);
    if (Dg::IsZero(length))
        a_axis.Zero();
    else
    {
        length = static_cast<Real>(1.0) / length;
        a_axis.m_V[0] = m_x * length;
        a_axis.m_V[1] = m_y * length;
        a_axis.m_V[2] = m_z * length;
        a_axis.m_V[3] = static_cast<Real>(0.0);
    }
}

//-------------------------------------------------------------------------------
//    @   Quaternion::GetBasis()
//-------------------------------------------------------------------------------
template<typename Real> void Quaternion<Real>::GetBasis(Vector4<Real>& a_x0, Vector4<Real>& a_x1, Vector4<Real>& a_x2) const
{
    Real xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;
    xs = m_x + m_x;
    ys = m_y + m_y;
    zs = m_z + m_z;
    wx = m_w * xs;
    wy = m_w * ys;
    wz = m_w * zs;
    xx = m_x * xs;
    xy = m_x * ys;
    xz = m_x * zs;
    yy = m_y * ys;
    yz = m_y * zs;
    zz = m_z * zs;
    a_x0.m_V[0] = static_cast<Real>(1.0) - (yy + zz);
    a_x0.m_V[1]= xy + wz;
    a_x0.m_V[2] = xz - wy;
    a_x0.m_V[3] = static_cast<Real>(0.0);
    a_x1.m_V[0] = xy - wz;
    a_x1.m_V[1] = static_cast<Real>(1.0) - (xx + zz);
    a_x1.m_V[2] = yz + wx;
    a_x1.m_V[3] = static_cast<Real>(0.0);
    a_x2.m_V[0] = xz + wy;
    a_x2.m_V[1] = yz - wx;
    a_x2.m_V[2] = static_cast<Real>(1.0) - (xx + yy);
    a_x2.m_V[3] = static_cast<Real>(0.0);
}

//-------------------------------------------------------------------------------
//    @   Quaternion::Clean()
//-------------------------------------------------------------------------------
template<typename Real> void Quaternion<Real>::Clean()
{
    if (Dg::IsZero(m_w)) m_w = static_cast<Real>(0.0);
    if (Dg::IsZero(m_x)) m_x = static_cast<Real>(0.0);
    if (Dg::IsZero(m_y)) m_y = static_cast<Real>(0.0);
    if (Dg::IsZero(m_z)) m_z = static_cast<Real>(0.0);
}

//-------------------------------------------------------------------------------
//    @   Quaternion::Normalize()
//-------------------------------------------------------------------------------
template<typename Real> void Quaternion<Real>::Normalize()
{
    Real lengthsq = m_w * m_w + m_x * m_x + m_y * m_y + m_z * m_z;

    if (Dg::IsZero(lengthsq))
        Zero();
    else
    {
        Real factor = static_cast<Real>(1.0) / Real(sqrt(lengthsq));
        m_w *= factor;
        m_x *= factor;
        m_y *= factor;
        m_z *= factor;
    }
}

//-------------------------------------------------------------------------------
//    @   Quaternion::Conjugate()
//-------------------------------------------------------------------------------
template<typename Real> Quaternion<Real> Conjugate(const Quaternion<Real>& a_quat)
    { return Quaternion<Real>(a_quat.m_w, -a_quat.m_x, -a_quat.m_y, -a_quat.m_z); }

//-------------------------------------------------------------------------------
//    @   Quaternion::Conjugate()
//-------------------------------------------------------------------------------
template<typename Real> const Quaternion<Real>& Quaternion<Real>::Conjugate()
{
    m_x = -m_x;
    m_y = -m_y;
    m_z = -m_z;
    return *this;
}

//-------------------------------------------------------------------------------
//    @   Quaternion::Inverse()
//-------------------------------------------------------------------------------
template<typename Real> Quaternion<Real> Inverse(const Quaternion<Real>& a_quat)
{
    Quaternion<Real> q(a_quat);
    q.Inverse();
    return q;
}

//-------------------------------------------------------------------------------
//    @   Quaternion::Inverse()
//-------------------------------------------------------------------------------
template<typename Real> const Quaternion<Real>& Quaternion<Real>::Inverse()
{
    Real norm = (m_w * m_w) + (m_x * m_x) + (m_y * m_y) + (m_z * m_z);
    if (Dg::IsZero(norm))                                           // if we're the zero quaternion, just return
        return *this;

    Real normRecip = static_cast<Real>(1.0) / norm;
    m_w = normRecip * m_w;
    m_x = -normRecip * m_x;
    m_y = -normRecip * m_y;
    m_z = -normRecip * m_z;
    return *this;
}

//-------------------------------------------------------------------------------
//    @   Quaternion::operator+()
//-------------------------------------------------------------------------------
template<typename Real> Quaternion<Real> Quaternion<Real>::operator + (const Quaternion<Real>& a_other) const
{
    return Quaternion<Real>(m_w + a_other.m_w, m_x + a_other.m_x, 
                            m_y + a_other.m_y, m_z + a_other.m_z);
}

//-------------------------------------------------------------------------------
//    @   Quaternion::operator+=()
//-------------------------------------------------------------------------------
template<typename Real> Quaternion<Real>& Quaternion<Real>::operator += (const Quaternion<Real>& a_other)
{
    m_w += a_other.m_w;
    m_x += a_other.m_x;
    m_y += a_other.m_y;
    m_z += a_other.m_z;
    return *this;
}

//-------------------------------------------------------------------------------
//    @   Quaternion::operator-()
//-------------------------------------------------------------------------------
template<typename Real> Quaternion<Real> Quaternion<Real>::operator - (const Quaternion<Real>& a_other) const
{
    return Quaternion<Real>(m_w - a_other.m_w, m_x - a_other.m_x, 
                            m_y - a_other.m_y, m_z - a_other.m_z);
}

//-------------------------------------------------------------------------------
//    @   Quaternion::operator-=()
//-------------------------------------------------------------------------------
template<typename Real> Quaternion<Real>& Quaternion<Real>::operator -= (const Quaternion<Real>& a_other)
{
    m_w -= a_other.m_w;
    m_x -= a_other.m_x;
    m_y -= a_other.m_y;
    m_z -= a_other.m_z;
    return *this;
}

//-------------------------------------------------------------------------------
//    @   Quaternion::operator-()
//-------------------------------------------------------------------------------
template<typename Real> Quaternion<Real> Quaternion<Real>::operator - () const
    { return Quaternion<Real>(-m_w, -m_x, -m_y, -m_z); }

//-------------------------------------------------------------------------------
//    @   Quaternion::operator*()
//-------------------------------------------------------------------------------
template<typename Real> Quaternion<Real> operator * (Real a_scalar, const Quaternion<Real>& a_quat)
{
    return Quaternion<Real>(a_scalar * a_quat.m_w, a_scalar * a_quat.m_x, 
                            a_scalar * a_quat.m_y, a_scalar * a_quat.m_z);
}

//-------------------------------------------------------------------------------
//    @   Quaternion::operator*=()
//-------------------------------------------------------------------------------
template<typename Real> Quaternion<Real>& Quaternion<Real>::operator *= (Real a_scalar)
{
    m_w *= a_scalar;
    m_x *= a_scalar;
    m_y *= a_scalar;
    m_z *= a_scalar;
    return *this;
}

//-------------------------------------------------------------------------------
//    @   Quaternion::operator*()
//-------------------------------------------------------------------------------
template<typename Real> Quaternion<Real> Quaternion<Real>::operator * (const Quaternion<Real>& a_other) const
{
  return Quaternion<Real>(
    a_other.m_w*m_w - a_other.m_x*m_x - a_other.m_y*m_y - a_other.m_z*m_z,
    a_other.m_w*m_x + a_other.m_x*m_w + a_other.m_y*m_z - a_other.m_z*m_y,
    a_other.m_w*m_y + a_other.m_y*m_w + a_other.m_z*m_x - a_other.m_x*m_z,
    a_other.m_w*m_z + a_other.m_z*m_w + a_other.m_x*m_y - a_other.m_y*m_x);

}   // End of Quaternion::operator*()


//-------------------------------------------------------------------------------
//    @   Quaternion::operator*=()
//-------------------------------------------------------------------------------
template<typename Real> Quaternion<Real>& Quaternion<Real>::operator *= (const Quaternion<Real>& a_other)
{
    Set(a_other.m_w*m_w - a_other.m_x*m_x - a_other.m_y*m_y - a_other.m_z*m_z,
        a_other.m_w*m_x + a_other.m_x*m_w + a_other.m_y*m_z - a_other.m_z*m_y,
        a_other.m_w*m_y + a_other.m_y*m_w + a_other.m_z*m_x - a_other.m_x*m_z,
        a_other.m_w*m_z + a_other.m_z*m_w + a_other.m_x*m_y - a_other.m_y*m_x);
    return *this;
}

//-------------------------------------------------------------------------------
//    @   Dot()
//-------------------------------------------------------------------------------
template<typename Real> Real Dot(const Quaternion<Real>& a_quat1, const Quaternion<Real>& a_quat2)
{
    return (a_quat1.m_w * a_quat2.m_w + a_quat1.m_x * a_quat2.m_x +
            a_quat1.m_y * a_quat2.m_y + a_quat1.m_z * a_quat2.m_z);
}

//-------------------------------------------------------------------------------
//    @   Quaternion::Rotate()
//-------------------------------------------------------------------------------
template<typename Real> Vector4<Real> Quaternion<Real>::Rotate(const Vector4<Real>& a_vector) const
{
    Real vMult = static_cast<Real>(2.0) * (m_x * a_vector.m_V[0] + m_y * a_vector.m_V[1] + m_z * a_vector.m_V[2]);
    Real crossMult = static_cast<Real>(2.0) * m_w;
    Real pMult = crossMult * m_w - static_cast<Real>(1.0);
    Vector4<Real> result;
    result.m_V[0] = pMult * a_vector.m_V[0] + vMult * m_x + crossMult * (m_y * a_vector.m_V[2] - m_z * a_vector.m_V[1]);
    result.m_V[1] = pMult * a_vector.m_V[1] + vMult * m_y + crossMult * (m_z * a_vector.m_V[0] - m_x * a_vector.m_V[2]);
    result.m_V[2] = pMult * a_vector.m_V[2] + vMult * m_z + crossMult * (m_x * a_vector.m_V[1] - m_y * a_vector.m_V[0]);
    result.m_V[3] = a_vector.m_V[3];
    return result;
}

//-------------------------------------------------------------------------------
//    @   Quaternion::RotateSelf()
//-------------------------------------------------------------------------------
template<typename Real> void Quaternion<Real>::RotateSelf(Vector4<Real>& a_vector) const
{
    Real vMult = static_cast<Real>(2.0) * (m_x * a_vector.m_V[0] + m_y * a_vector.m_V[1] + m_z * a_vector.m_V[2]);
    Real crossMult = static_cast<Real>(2.0) * m_w;
    Real pMult = crossMult * m_w - static_cast<Real>(1.0);
    Real x = pMult * a_vector.m_V[0] + vMult * m_x + crossMult * (m_y * a_vector.m_V[2] - m_z * a_vector.m_V[1]);
    Real y = pMult * a_vector.m_V[1] + vMult * m_y + crossMult * (m_z * a_vector.m_V[0] - m_x * a_vector.m_V[2]);
    Real z = pMult * a_vector.m_V[2] + vMult * m_z + crossMult * (m_x * a_vector.m_V[1] - m_y * a_vector.m_V[0]);
    a_vector.m_V[0] = x;
    a_vector.m_V[1] = y;
    a_vector.m_V[2] = z;
}

//-------------------------------------------------------------------------------
//    @   Quaternion::Lerp()
//-------------------------------------------------------------------------------
template<typename Real> void Lerp(Quaternion<Real>& a_result, const Quaternion<Real>& a_start, const Quaternion<Real>& a_end, Real a_t)
{
    Real cosTheta = Dot(a_start, a_end);                            // get cos of "angle" between quaternions
    a_result = a_t * a_end;                                         // initialize a_result
    if (cosTheta >= static_cast<Real>(EPSILON_d))                   // if "angle" between quaternions is less than 90 degrees
        a_result += (static_cast<Real>(1.0) - a_t) * a_start;       // use standard interpolation
    else
        a_result += (a_t - static_cast<Real>(1.0)) * a_start;       // otherwise, take the shorter path
}

//-------------------------------------------------------------------------------
//    @   Quaternion::Slerp()
//-------------------------------------------------------------------------------
template<typename Real> void Slerp(Quaternion<Real>& a_result, const Quaternion<Real>& a_start, const Quaternion<Real>& a_end, Real a_t)
{
    Real cosTheta = Dot(a_start, a_end);                            // get cosine of "angle" between quaternions
    Real startInterp, endInterp;
  
    if (cosTheta >= static_cast<Real>(EPSILON_d))                   // if "angle" between quaternions is less than 90 degrees
    {                                                               // if angle is greater than zero
        if ((static_cast<Real>(1.0) - cosTheta) > static_cast<Real>(EPSILON_d)) 
        {
            Real theta = Real(acos(cosTheta));                      // use standard slerp
            Real recipSinTheta = static_cast<Real>(1.0) / Real(sin(theta));
            startInterp = Real(sin((static_cast<Real>(1.0) - a_t) * theta)) * recipSinTheta;
            endInterp = Real(sin(a_t*theta)) * recipSinTheta;
        }
        else                                                        // angle is close to zero
        {
            startInterp = static_cast<Real>(1.0) - a_t;             // use linear interpolation
            endInterp = a_t;
        }
    }
    else                                                            // otherwise, take the shorter route
    {                                                               // if angle is less than 180 degrees
        if ((static_cast<Real>(1.0) + cosTheta) > static_cast<Real>(EPSILON_d))
        {
            Real theta = Real(acos(-cosTheta));                     // use slerp w/negation of a_start quaternion
            Real recipSinTheta = static_cast<Real>(1.0) / Real(sin(theta));
            startInterp = Real(sin((a_t - static_cast<Real>(1.0)) * theta)) * recipSinTheta;
            endInterp = Real(sin(a_t * theta)) * recipSinTheta;
        }
        else                                                        // angle is close to 180 degrees
        {
            startInterp = a_t - static_cast<Real>(1.0);             // use lerp w/negation of a_start quaternion
            endInterp = a_t;
        }
    }
    a_result = startInterp * a_start + endInterp * a_end;
}

//-------------------------------------------------------------------------------
//    @   Quaternion::ApproxSlerp()
//-------------------------------------------------------------------------------
template<typename Real> void ApproxSlerp(Quaternion<Real>& a_result, const Quaternion<Real>& a_start, const Quaternion<Real>& a_end, Real a_t)
{
    Real cosTheta = Dot(a_start, a_end);
    Real factor = static_cast<Real>(1.0) - static_cast<Real>(0.7878088) * cosTheta;
    Real k = static_cast<Real>(0.5069269);
    factor *= factor;
    k *= factor;
    Real b = static_cast<Real>( 2.0) * k;
    Real c = static_cast<Real>(-3.0) * k;
    Real d = static_cast<Real>( 1.0) + k;
    a_t = a_t * (b * a_t + c) + d;
    a_result = a_t * a_end;                                         // initialize a_result
    if (cosTheta >= static_cast<Real>(EPSILON_d))                   // if "angle" between quaternions is less than 90 degrees
        a_result += (static_cast<Real>(1.0) - a_t) * a_start;       // use standard interpolation
    else
        a_result += (a_t - static_cast<Real>(1.0)) * a_start;       // otherwise, take the shorter path
}

}

#endif
