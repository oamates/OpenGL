#ifndef _cms_face_included_5918463059816203857612035876104561384756138974561394
#define _cms_face_included_5918463059816203857612035876104561384756138974561394

#include "cell.hpp"

namespace cms
{

//========================================================================================================================================================================================================================
// strip structure :: holds indices to two vertices and two face edges
//========================================================================================================================================================================================================================

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

//========================================================================================================================================================================================================================
// BRANCH_FACE may also means EMPTY
//========================================================================================================================================================================================================================
enum FaceState 
{ 
    BRANCH_FACE  = 0,
    LEAF_FACE    = 1,
    TRANSIT_FACE = 2
};

//========================================================================================================================================================================================================================
// face structure :: stores information about cell faces such as data on the face and it's belonging cell
//========================================================================================================================================================================================================================

struct face_t
{
    bool skip;                              // a flag denoting whether the face has been taken care of
    FaceState state;                        // the state enumerator of the face
    face_t* twin;                           // a ptr to the face's twin face
    face_t* parent;                         // a ptr to the face's parent face
    face_t* children[4];                    // an array of 4 ptrs of the face's child face's
    std::vector<strip_t> strips;            // a vector of strips on that face

    std::vector<std::vector<unsigned int>> transit_segments;

    face_t()
        : skip(true), state(BRANCH_FACE), twin(0), parent(0)
    {
        for(int i = 0; i < 4; ++i)
            children[i] = 0;
    }
};


} // namespace cms

#endif // _cms_face_included_5918463059816203857612035876104561384756138974561394
