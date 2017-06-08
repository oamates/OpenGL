#ifndef _cms_strip_included_158273456437850624378650234765243786578436578436587
#define _cms_strip_included_158273456437850624378650234765243786578436578436587

#include "index3d.hpp"

namespace cms
{

// Strip structure :: holding indices to two vertices and two face edges

struct Strip
{
    bool skip;
    bool loop;
    int edge[2];            // local cell edge indices
    int data[2];            // global datastruct edge indices

    Index3D block[2];       // 3D Index of the EdgeBlock into the global datastruct
    int dir[2];             // Direction of the edge on the edge block

    Strip()
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
    Strip(bool _skip, int _edge0, int  _edge1)
        : skip(_skip), loop(false)
    {
        edge[0] = _edge0;
        edge[1] = _edge1;
        data[0] = -1;
        data[1] = -1;
        dir[0] = -1;
        dir[1] = -1;
    }

    // i is the last
    void changeBack(Strip& s, int i)
    {
        edge[1] = s.edge[i];
        data[1] = s.data[i];
        dir[1] = s.dir[i];
        block[1] = s.block[i];
    }

    // i is the first
    void changeFront(Strip& s, int i)
    {
        edge[0] = s.edge[i];
        data[0] = s.data[i];
        dir[0] = s.dir[i];
        block[0] = s.block[i];
    }

};


} // namespace cms

#endif // _cms_strip_included_158273456437850624378650234765243786578436578436587