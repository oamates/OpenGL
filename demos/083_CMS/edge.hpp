#ifndef _cms_edge_included_9143684378567835672065123560781235687346587431658714
#define _cms_edge_included_9143684378567835672065123560781235687346587431658714

#include "index3d.hpp"

namespace cms
{

//=======================================================================================================================================================================================================================
// structure edge_block_t holding 3 edges with ordering x y z - right up front
//=======================================================================================================================================================================================================================

struct edge_block_t
{
    bool empty;                                                 // the empty bool flag
    Index3D edge_indices;                                       // the 3d edgeblock index in the edge array

    edge_block_t()
        : empty(true), edge_indices(-1)
    {}

    edge_block_t(bool empty)
        : empty(empty), edge_indices(-1)
    {}

    edge_block_t(int right, int up, int front)
        : empty(false), edge_indices(right, up, front)
    {}

    edge_block_t(bool empty, int right, int up, int front)
        : empty(empty), edge_indices(right, up, front)
    {}
  
    int& operator[] (const int& index)                          // Overloaded [] operator
        { return (&edge_indices.m_x)[index]; }

};

} // namespace cms

#endif // _cms_edge_included_9143684378567835672065123560781235687346587431658714