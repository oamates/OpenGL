#ifndef _cms_edge_included_9143684378567835672065123560781235687346587431658714
#define _cms_edge_included_9143684378567835672065123560781235687346587431658714

#include <glm/glm.hpp>

namespace cms
{

//=======================================================================================================================================================================================================================
// structure edge_block_t holding 3 edges with ordering x y z - right up front
//=======================================================================================================================================================================================================================

struct edge_block_t
{
    bool empty;                                                 // the empty bool flag
    glm::ivec3 edge_indices;                                    // the 3d edgeblock index in the edge array

    edge_block_t()
        : empty(true), edge_indices(-1)
    {}

    int& operator[] (const int& index)                          // Overloaded [] operator
        { return edge_indices[index]; }

};

} // namespace cms

#endif // _cms_edge_included_9143684378567835672065123560781235687346587431658714