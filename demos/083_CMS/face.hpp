#ifndef _cms_face_included_5918463059816203857612035876104561384756138974561394
#define _cms_face_included_5918463059816203857612035876104561384756138974561394

#include "cell.hpp"
#include "vec3.hpp"
#include "util.hpp"
#include "strip.hpp"

namespace cms
{

// BRANCH_FACE may also means EMPTY
enum FaceState 
{ 
    BRANCH_FACE  = 0,
    LEAF_FACE    = 1,
    TRANSIT_FACE = 2
};

// A Face structure stores information about cell faces such as data on the face and it's belonging cell
// todo :: optimise the transitSegs vec of vec structure

struct face_t
{
    int id;                                 // the unique identifier of the face, todo :: only used for debug
    int cell_index;                         // the unique identifier of the face's cell
    bool skip;                              // a flag denoting whether the face has been taken care of
    FaceState state;                        // the state enumerator of the face
    Vec3 featurePosition;                   // the exact face sharp feature position in 3d space, todo :: to be used
    face_t* twin;                           // a ptr to the face's twin face
    face_t* parent;                         // a ptr to the face's parent face
    face_t* children[4];                    // an array of 4 ptrs of the face's child face's
    std::vector<strip_t> strips;            // a vector of strips on that face

    // an Array of transitional segments on that face, if it was a transitional face, it would have a twin with more
    // a more complex set of strips, it stores them here when it has collected them
    std::vector<std::vector<unsigned int>> transitSegs;

    face_t(int id, int cell_index)
        : id(id), cell_index(cell_index), 
          skip(true), state(BRANCH_FACE), /*sharpFeature(false),*/ featurePosition(Vec3(0.0f)), twin(0), parent(0)
    {
        for(int i = 0; i < 4; ++i)
            children[i] = 0;
    }
};


} // namespace cms

#endif // _cms_face_included_5918463059816203857612035876104561384756138974561394
