#ifndef _cms_array3d_included_8912356198237561723546982375912735692356785691253
#define _cms_array3d_included_8912356198237561723546982375912735692356785691253

#include <cmath>
#include <vector>
#include "vec3.hpp"
#include "index3d.hpp"
#include "range.hpp"
#include "edge.hpp"

namespace cms
{

// Array3D :: wrapper for a 3D array, used for 'float' and 'EdgeBlock' types, in the sample and edge array

template <class T> struct Array3D
{
    std::vector<T> m_data;                                                              // The dynamic 1D array of the actual data
    Index3D m_indices;                                                                  // An Index3D storing the size of the 3D array wrapper in X, Y and Z
    Range m_bbox[3];                                                                    // The BBox of the Array3D stored as an array of 3 Ranges (-x +x), (-y, +y), (-z, +z)
    
    Array3D()                                                                           // Empty constructor - sets bbox to (-1,1) in xyz
    {
        for(int i = 0; i < 3; ++i)
        {
            m_indices[i] = 1;
            m_bbox[i] = Range(-1.0f, 1.0f);
        }
    }

    Array3D(Range bbox[3], int xSlab, int ySlab, int zSlab)                            // Full constructor, taking the bbox and the samples - resizing the array
    {
        for(int i = 0; i < 3; ++i)
            m_bbox[i] = bbox[i];

        m_indices.m_x = xSlab;
        m_indices.m_y = ySlab;
        m_indices.m_z = zSlab;
        m_data.resize(m_indices.m_x * m_indices.m_y * m_indices.m_z);
    }

    ~Array3D() {}


    void operator() (const int x, const int y, const int z, T value)
    {
        assert(x < m_indices.m_x);
        assert(y < m_indices.m_y);
        assert(z < m_indices.m_z);
        m_data[(x * m_indices.m_y + y) * m_indices.m_z + z] = value;
    }

    
    int size() const                                                                            // Returns the size of the whole flattened Array3D
        { return m_data.size(); }

    // Used to set the value at a specific position in the Array3D, params :: three integers denoting the location, and a value of type T
    void setValueAt(int x, int y, int z, T value)
    {
        assert(x < m_indices.m_x);
        assert(y < m_indices.m_y);
        assert(z < m_indices.m_z);

        m_data[(x * m_indices.m_y + y) * m_indices.m_z + z] = value;
    }

    // Used to set the value at a specific position in the Array3D, params :: an Index3D denoting the location, and a value of type T
    void setValueAt(Index3D xyz, T value)
    {
        assert(xyz.m_x < m_indices.m_x);
        assert(xyz.m_y < m_indices.m_y);
        assert(xyz.m_z < m_indices.m_z);
        m_data[(xyz.m_x * m_indices.m_y + xyz.m_y) * m_indices.m_z + xyz.m_z] = value;
    }

    // Returns the value from a specific position in the Array3D, params :: three integers denoting the location
    T getValueAt(int x, int y, int z) const
    {
        assert(x < m_indices.m_x);
        assert(y < m_indices.m_y);
        assert(z < m_indices.m_z);
        return m_data[(x * m_indices.m_y + y) * m_indices.m_z + z];
    }

    // Returns the value from a specific position in the Array3D, params :: an Index3D denoting the location
    T getValueAt(Index3D xyz) const
    {
        assert(xyz.m_x < m_indices.m_x); 
        assert(xyz.m_y < m_indices.m_y); 
        assert(xyz.m_z < m_indices.m_z);
        return m_data[(xyz.m_x * m_indices.m_y + xyz.m_y) * m_indices.m_z + xyz.m_z];
    }

    // Returning the flattened index in the actual 1D array, params :: Three integers denoting the location in the Array3D
    int getIndexAt(int x, int y, int z) const
    {
        assert(x < m_indices.m_x);
        assert(y < m_indices.m_y);
        assert(z < m_indices.m_z);
        return (x * m_indices.m_y + y) * m_indices.m_z + z;
    }

    // Returning the flattened index in the actual 1D array, params :: an Index3D denoting the location in the Array3D
    int getIndexAt(Index3D xyz) const
    {
        assert(xyz.m_x < m_indices.m_x);
        assert(xyz.m_y < m_indices.m_y);
        assert(xyz.m_z < m_indices.m_z);
        return (xyz.m_x * m_indices.m_y + xyz.m_y) * m_indices.m_z + xyz.m_z;
    }

    // Returns a Vec3 of the exact position in 3D space of a specific sample, params :: three integers denoting the location of the sample in the Array3D
    Vec3 getPositionAt(int x, int y, int z) const
    {
        Vec3 pos;

        const float tx = static_cast<float>(x) / static_cast<float>(m_indices.m_x - 1);
        pos.m_x = m_bbox[0].m_lower + (m_bbox[0].m_upper - m_bbox[0].m_lower) * tx;
        const float ty = static_cast<float>(y) / static_cast<float>(m_indices.m_y - 1);
        pos.m_y = m_bbox[1].m_lower + (m_bbox[1].m_upper - m_bbox[0].m_lower)*ty;
        const float tz = static_cast<float>(z) / static_cast<float>(m_indices.m_z - 1);
        pos.m_z = m_bbox[2].m_lower + (m_bbox[2].m_upper - m_bbox[0].m_lower)*tz;

        assert((pos.m_x >= m_bbox[0].m_lower) && (pos.m_x <= m_bbox[0].m_upper));
        assert((pos.m_y >= m_bbox[1].m_lower) && (pos.m_y <= m_bbox[1].m_upper));
        assert((pos.m_z >= m_bbox[2].m_lower) && (pos.m_z <= m_bbox[2].m_upper));

        return pos;
    }

    // Returns a Vec3 of the exact position in 3D space of a specific sample, params :: An Index3D denoting the location of the sample in the Array3D
    Vec3 getPositionAt(Index3D xyz) const
    {
        Vec3 pos;
        
        const float tx = static_cast<float>(xyz.m_x) / static_cast<float>(m_indices.m_x - 1);
        pos.m_x = m_bbox[0].m_lower + (m_bbox[0].m_upper - m_bbox[0].m_lower) * tx;
        
        const float ty = static_cast<float>(xyz.m_y) / static_cast<float>(m_indices.m_y - 1);
        pos.m_y = m_bbox[1].m_lower + (m_bbox[1].m_upper - m_bbox[0].m_lower) * ty;
        
        const float tz = static_cast<float>(xyz.m_z) / static_cast<float>(m_indices.m_z - 1);
        pos.m_z = m_bbox[2].m_lower + (m_bbox[2].m_upper - m_bbox[0].m_lower) * tz;
        
        assert((pos.m_x >= m_bbox[0].m_lower)&&(pos.m_x <= m_bbox[0].m_upper));
        assert((pos.m_y >= m_bbox[1].m_lower)&&(pos.m_y <= m_bbox[1].m_upper));
        assert((pos.m_z >= m_bbox[2].m_lower)&&(pos.m_z <= m_bbox[2].m_upper));
        
        return pos;
    }

    // Resize the Array3D, params :: three integer values corresponding to X, Y and Z
    void resize(int xSlab, int ySlab, int zSlab)
    {
        m_indices.m_x = xSlab;
        m_indices.m_y = ySlab;
        m_indices.m_z = zSlab;
        m_data.resize(m_indices.m_x * m_indices.m_y * m_indices.m_z);
    }

    // Resize the Array3D, params :: an Index3D giving the sizes in X, Y and Z
    void resize(Index3D slabs)
    {
        m_indices.m_x = slabs.m_x;
        m_indices.m_y = slabs.m_y;
        m_indices.m_z = slabs.m_z;
        m_data.resize(m_indices.m_x * m_indices.m_y * m_indices.m_z);
    }

    // Setting the BBox of the Array3D, params :: an array of ranges (should be size 3) with the min and max values in X, Y and Z
    void setBBox(Range bbox[])
    {
        for(int i = 0; i < 3; ++i)
            m_bbox[i] = bbox[i];
    }

};

template class Array3D<float>;
template class Array3D<edge_block_t>;

} // namespace cms

#endif // _cms_array3d_included_8912356198237561723546982375912735692356785691253