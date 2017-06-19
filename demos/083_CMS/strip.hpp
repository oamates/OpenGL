#ifndef _cms_strip_included_158273456437850624378650234765243786578436578436587
#define _cms_strip_included_158273456437850624378650234765243786578436578436587

#include <glm/glm.hpp>

namespace cms
{

// strip structure :: holding indices to two vertices and two face edges

struct strip_t
{
    bool skip;
    bool loop;
    int edge[2];                    // local cell edge indices
    int data[2];                    // global datastruct edge indices

    glm::ivec3 block[2];            // 3D Index of the EdgeBlock into the global datastruct
    int dir[2];                     // Direction of the edge on the edge block

    strip_t()
        : skip(true), loop(false)
    {
        edge[0] = -1;
        edge[1] = -1;
        data[0] = -1;
        data[1] = -1;
        dir[0] = -1;
        dir[1] = -1;
    }

    // the skip boolean - set it to true if there is valid data on the strip
    // edge0 and edge1 are the two edges on which the strip lies
    strip_t(bool skip, int edge0, int edge1)
        : skip(skip), loop(false)
    {
        edge[0] = edge0;
        edge[1] = edge1;
        data[0] = -1;
        data[1] = -1;
        dir[0] = -1;
        dir[1] = -1;
    }

    void changeBack(strip_t& s, int last)
    {
        edge[1] = s.edge[last];
        data[1] = s.data[last];
        dir[1] = s.dir[last];
        block[1] = s.block[last];
    }

    void changeFront(strip_t& s, int first)
    {
        edge[0] = s.edge[first];
        data[0] = s.data[first];
        dir[0] = s.dir[first];
        block[0] = s.block[first];
    }

};


} // namespace cms

#endif // _cms_strip_included_158273456437850624378650234765243786578436578436587