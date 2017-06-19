#ifndef _cms_array3d_included_8912356198237561723546982375912735692356785691253
#define _cms_array3d_included_8912356198237561723546982375912735692356785691253

#include <cmath>
#include <vector>

#include <glm/glm.hpp>

#include "range.hpp"
#include "edge.hpp"

namespace cms
{

template<typename T> struct Array3D
{
    std::vector<T> data;                                                                // The dynamic 1D array of the actual data
    glm::ivec3 size;                                                                    // An Index3D storing the size of the 3D array wrapper in X, Y and Z
    Range m_bbox[3];                                                                    // The BBox of the Array3D stored as an array of 3 Ranges (-x +x), (-y, +y), (-z, +z)
    
    Array3D()                                                                           // Empty constructor - sets bbox to (-1,1) in xyz
    {
        for(int i = 0; i < 3; ++i)
        {
            size[i] = 1;
            m_bbox[i] = Range(-1.0f, 1.0f);
        }
    }

    Array3D(Range bbox[3], int xSlab, int ySlab, int zSlab)                            // Full constructor, taking the bbox and the samples - resizing the array
    {
        for(int i = 0; i < 3; ++i)
            m_bbox[i] = bbox[i];

        size.x = xSlab;
        size.y = ySlab;
        size.z = zSlab;
        data.resize(size.x * size.y * size.z);
    }

    ~Array3D() {}

    void operator() (const int x, const int y, const int z, T value)
    {
        data[(x * size.y + y) * size.z + z] = value;
    }

    void setValueAt(int x, int y, int z, T value)
    {
        data[size.z * (size.y * x + y) + z] = value;
    }

    void setValueAt(glm::ivec3 xyz, T value)
    {
        data[(xyz.x * size.y + xyz.y) * size.z + xyz.z] = value;
    }

    T getValueAt(int x, int y, int z) const
    {
        return data[(x * size.y + y) * size.z + z];
    }

    T getValueAt(glm::ivec3 xyz) const
    {
        return data[(xyz.x * size.y + xyz.y) * size.z + xyz.z];
    }

    int getIndexAt(int x, int y, int z) const
    {
        return (x * size.y + y) * size.z + z;
    }

    int getIndexAt(glm::ivec3 xyz) const
    {
        return (xyz.x * size.y + xyz.y) * size.z + xyz.z;
    }

    // Returns a Vec3 of the exact position in 3D space of a specific sample, params :: An Index3D denoting the location of the sample in the Array3D
    glm::vec3 getPositionAt(glm::ivec3 xyz) const
    {
        const float tx = static_cast<float>(xyz.x) / static_cast<float>(size.x - 1);
        const float ty = static_cast<float>(xyz.y) / static_cast<float>(size.y - 1);
        const float tz = static_cast<float>(xyz.z) / static_cast<float>(size.z - 1);

        return glm::vec3(m_bbox[0].m_lower + (m_bbox[0].m_upper - m_bbox[0].m_lower) * tx,
                         m_bbox[1].m_lower + (m_bbox[1].m_upper - m_bbox[0].m_lower) * ty,
                         m_bbox[2].m_lower + (m_bbox[2].m_upper - m_bbox[0].m_lower) * tz);
    }

    // Resize the Array3D, params :: three integer values corresponding to X, Y and Z
    void resize(int xSlab, int ySlab, int zSlab)
    {
        size.x = xSlab;
        size.y = ySlab;
        size.z = zSlab;
        data.resize(size.x * size.y * size.z);
    }

    // Resize the Array3D, params :: an Index3D giving the sizes in X, Y and Z
    void resize(glm::ivec3 slabs)
    {
        size.x = slabs.x;
        size.y = slabs.y;
        size.z = slabs.z;
        data.resize(size.x * size.y * size.z);
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