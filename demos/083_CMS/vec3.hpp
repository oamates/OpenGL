#ifndef _cms_vec3_included_0145670187568723507125638724508712307213516235620853
#define _cms_vec3_included_0145670187568723507125638724508712307213516235620853

#include <cmath>
#include <iostream>

namespace cms
{

// define EPSILON for floating point comparison
#ifndef EPSILON
    const static float EPSILON = 0.00001f;
#endif

// FCompare macro used for floating point comparision functions
#define FCompare(a,b) \
    (((a) - EPSILON) < (b) && ((a) + EPSILON) > (b))

struct Vec3
{
    float m_x, m_y, m_z;

    Vec3(const Vec3& _v) : m_x(_v.m_x), m_y(_v.m_y), m_z(_v.m_z) {}

    Vec3(float _x = 0.0, float _y = 0.0, float _z = 0.0) : m_x(_x), m_y(_y), m_z(_z) {}

    void set(float _x, float _y, float _z)
    {
        m_x = _x;
        m_y = _y;
        m_z = _z;
    }

    void set(const Vec3& _v)
    {
        m_x = _v.m_x;
        m_y = _v.m_y;
        m_z = _v.m_z;
    }

    void set(const Vec3* _v)
    {
        m_x = _v->m_x;
        m_y = _v->m_y;
        m_z = _v->m_z;
    }
  
    float dot(const Vec3& _v) const
    {
        return m_x * _v.m_x + m_y * _v.m_y + m_z * _v.m_z;
    }

    void cross(const Vec3& _v1, const Vec3& _v2 )
    {
        m_x = _v1.m_y * _v2.m_z - _v1.m_z * _v2.m_y;
        m_y = _v1.m_z * _v2.m_x - _v1.m_x * _v2.m_z;
        m_z = _v1.m_x * _v2.m_y - _v1.m_y * _v2.m_x;
    }

    Vec3 cross(const Vec3& _v )const
    {
        return Vec3(
            m_y * _v.m_z - m_z * _v.m_y,
            m_z * _v.m_x - m_x * _v.m_z,
            m_x * _v.m_y - m_y * _v.m_x
        );
    }

    void null()
    {
        m_x = 0.0f;
        m_y = 0.0f;
        m_z = 0.0f;
    }
    
    float& operator[] (const int& _i)
    {
        return (&m_x)[_i];
    }

    float length() const
        { return sqrt(m_x * m_x + m_y * m_y + m_z * m_z); }

    float lengthSquared() const
        { return m_x * m_x + m_y * m_y + m_z * m_z; }

    void normalize()
    {
        float l = sqrt(m_x * m_x + m_y * m_y + m_z * m_z);
        m_x /= l;
        m_y /= l;
        m_z /= l;
    }
  
    float inner(const Vec3& _v) const
        { return m_x * _v.m_x + m_y * _v.m_y + m_z * _v.m_z; }
  
    Vec3 outer(const Vec3& _v) const
    {
        float x = (m_y * _v.m_z) - (m_z * _v.m_y);
        float y = (m_z * _v.m_x) - (m_x * _v.m_z);
        float z = (m_x * _v.m_y) - (m_y * _v.m_x);
        return Vec3(x, y, z);
    }
  
    void operator += (const Vec3& _v)
    {
        m_x += _v.m_x;
        m_y += _v.m_y;
        m_z += _v.m_z;
    }

    void operator -= (const Vec3& _v)
    {
        m_x -= _v.m_x;
        m_y -= _v.m_y;
        m_z -= _v.m_z;
    }

    Vec3 operator * (float _i) const
        { return Vec3(m_x * _i, m_y * _i, m_z * _i); }

    Vec3 operator + (const Vec3 &_v) const
        { return Vec3(m_x + _v.m_x, m_y + _v.m_y, m_z + _v.m_z); }

    Vec3 operator / (float _v) const
        { return Vec3(m_x / _v, m_y / _v, m_z / _v); }

    Vec3 operator / (const Vec3& _v) const
        { return Vec3(m_x / _v.m_x, m_y / _v.m_y, m_z / _v.m_z); }

    void operator /= (float _v)
    {
        m_x /= _v;
        m_y /= _v;
        m_z /= _v;
    }

    void operator *= (float _v)
    {
        m_x *= _v;
        m_y *= _v;
        m_z *= _v;
    }
  
    Vec3 operator - (const Vec3 &_v) const
        { return Vec3(m_x - _v.m_x, m_y - _v.m_y, m_z - _v.m_z); }

    Vec3 operator - () const
        { return Vec3(-m_x, -m_y, -m_z); }

    Vec3 operator * (const Vec3 &_v) const
        { return Vec3(m_x * _v.m_x, m_y * _v.m_y, m_z * _v.m_z); }

    Vec3& operator = (const Vec3 &_v)
    {
        m_x = _v.m_x;
        m_y = _v.m_y;
        m_z = _v.m_z;
        return *this;
    }

    Vec3 &operator = (float _v)
    {
        m_x = _v;
        m_y = _v;
        m_z = _v;
        return *this;
    }

    bool operator == (const Vec3 &_v) const
    {
        return FCompare(_v.m_x, m_x) && FCompare(_v.m_y, m_y) && FCompare(_v.m_z, m_z);
    }
  
    bool operator != (const Vec3 &_v) const
    {
        return !FCompare(_v.m_x,m_x) || !FCompare(_v.m_y,m_y) || !FCompare(_v.m_z,m_z);
    }

};

inline std::ostream& operator << (std::ostream& _output, const Vec3& _v)
{
    return _output << "[" << _v.m_x << "," << _v.m_y << "," << _v.m_z << "]";
}

inline std::istream& operator >> (std::istream& _input, Vec3 &_s)
{
    return _input >> _s.m_x >> _s.m_y >> _s.m_z;
}

} // namespace cms

#endif // _cms_vec3_included_0145670187568723507125638724508712307213516235620853
