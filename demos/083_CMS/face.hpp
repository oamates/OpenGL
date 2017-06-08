#ifndef _cms_face_included_5918463059816203857612035876104561384756138974561394
#define _cms_face_included_5918463059816203857612035876104561384756138974561394

#include "cell.hpp"
#include "vec3.hpp"
#include "util.hpp"
#include "strip.hpp"

namespace cms
{

// Face State enumerator
/// BRANCH_FACE may also mean EMPTY
enum FaceState 
{ 
    BRANCH_FACE  = 0,
    LEAF_FACE    = 1,
    TRANSIT_FACE = 2
};

/// @struct Face
// A Face structure stores information about
/// cell faces such as data on the face and it's belonging cell
/// @todo optimise the transitSegs vec of vec structure
///
struct Face
{
    // Face struct typedefs
    typedef std::vector<Strip> StripVec;

    // Full Contructor
  /// @param int _id - the unique identifier of the face
  /// @param int _cellInd - the unique identifier of it's cell
  /// @param sets all other members to a default value
             Face(int _id, int _cellInd) :
                id(_id)
              , cellInd(_cellInd)
              , skip(true)
              , state(BRANCH_FACE)
              , sharpFeature(false)
              , featurePosition(Vec3(0))
              , twin(NULL)
              , parent(NULL)
              {
                for(int i=0;i<4;++i)
                  children[i] = nullptr;
              }

  // the unique identifier of the face
  int        id; //todo only used for debug

  // the unique identifier of the face's cell
  int        cellInd;

  // a flag denoting whether the face has been taken care of
  bool       skip;

  // the state enumerator of the face
  FaceState  state;

  // a flag for face sharp features
  /// todo: to be used
  bool       sharpFeature;

  // the exact face sharp feature position in 3d space
  /// todo: to be used
  Vec3       featurePosition;

  // a ptr to the face's twin face
  Face*      twin;

  // a ptr to the face's parent face
  Face*      parent;

  // an array of 4 ptrs of the face's child face's
  Face*      children[4];

  // a vector of strips on that face
  StripVec   strips;

  // an Array of transitional segments on that face
  /// if it was a transitional face, it would have a twin with more
  /// a more complex set of strips, it stores them here when it has
  /// collected them
  /// @todo: fix the vec of vec
  std::vector<std::vector<unsigned int>> transitSegs;
};


} // namespace cms

#endif // _cms_face_included_5918463059816203857612035876104561384756138974561394
