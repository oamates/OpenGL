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
    std::vector<T> data;                                                            // The dynamic 1D array of the actual data
    int size;                                                                       // An Index3D storing the size of the 3D array wrapper in X, Y and Z
    
    Array3D() {}                                                                        

    ~Array3D() {}

    void resize(int size)
    {
        Array3D::size = size;
        data.resize(size * size * size);
    }

    int linear_index(glm::ivec3 index3d) const
        { return size * (size * index3d.x + index3d.y) + index3d.z; }

    T& operator[] (const glm::ivec3& index3d)
        { return data[linear_index(index3d)]; }

    const T& operator[] (const glm::ivec3& index3d) const
        { return data[linear_index(index3d)]; }
};

template struct Array3D<float>;
template struct Array3D<edge_block_t>;

} // namespace cms

#endif // _cms_array3d_included_8912356198237561723546982375912735692356785691253