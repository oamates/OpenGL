#ifndef _cms_index3d_included_9824357160724356017463501827659134756843756814375
#define _cms_index3d_included_9824357160724356017463501827659134756843756814375

#include <iostream>

namespace cms
{

// Index3D, serving as a vec3 for integer values

struct Index3D
{
    int m_x;
    int m_y;
    int m_z;

    Index3D()
        : m_x(-1), m_y(-1), m_z(-1)
    {}
    
    Index3D(const int _i)
        : m_x(_i), m_y(_i), m_z(_i)
    {}
    
    Index3D(const Index3D& _ind)
        : m_x(_ind.m_x), m_y(_ind.m_y), m_z(_ind.m_z)
    {}
    
    Index3D(const int _x, const int _y, const int _z)
        : m_x(_x), m_y(_y), m_z(_z)
    {}

    int& operator[] (const int& _i)
        { return (&m_x)[_i]; }
    
    Index3D& operator = (const Index3D& i_ind)
    {
        m_x = i_ind.m_x;
        m_y = i_ind.m_y;
        m_z = i_ind.m_z;
        return *this;
    }
    
    Index3D operator - (const int& _i)
    {
        Index3D temp(*this);
        temp.m_x -= _i;
        temp.m_y -= _i;
        temp.m_z -= _i;
        return temp;
    }
    
    Index3D operator - (const Index3D& _ind)
    {
        Index3D temp(*this);
        temp.m_x -= _ind.m_x;
        temp.m_y -= _ind.m_y;
        temp.m_z -= _ind.m_z;
        return temp;
    }
    
    Index3D operator + (const int& _i)
    {
        Index3D temp(*this);
        temp.m_x += _i;
        temp.m_y += _i;
        temp.m_z += _i;
        return temp;
    }
    
    Index3D operator + (const Index3D& _ind)
    {
        Index3D temp(*this);
        temp.m_x += _ind.m_x;
        temp.m_y += _ind.m_y;
        temp.m_z += _ind.m_z;
        return temp;
    }
    
    void operator -= (const int& _i)
    {
        m_x -= _i;
        m_y -= _i;
        m_z -= _i;
    }
    
    void operator += (const int& _i)
    {
        m_x += _i;
        m_y += _i;
        m_z += _i;
    }
    
    bool operator == (const Index3D &_ind) const
        { return (_ind.m_x == m_x) && (_ind.m_y == m_y) && (_ind.m_z == m_z); }
    
    bool operator != (const Index3D &_ind) const
        { return (_ind.m_x != m_x) || (_ind.m_y != m_y) || (_ind.m_z != m_z); }

};

// Overloading the steaming operator so that a Index3D could be printed with correct formatting

inline std::ostream& operator << (std::ostream& _output, const Index3D& _i)
    { return _output << "[" << _i.m_x << ", " << _i.m_y << ", " << _i.m_z << "]"; }

} // namespace cms

#endif // _cms_index3d_included_9824357160724356017463501827659134756843756814375