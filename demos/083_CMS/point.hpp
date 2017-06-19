#ifndef _cms_point_included_243906370634075863405634576437563978456783457813451
#define _cms_point_included_243906370634075863405634576437563978456783457813451

#include "vec3.hpp"

namespace cms
{

// Stores a position in space, it's function value and gradient
struct Point
{
    Vec3 position;
    float value;

    Point(Vec3 position, float value) 
        : position(position), value(value)
    {}

    ~Point() {};

};


} // namespace cms

#endif // _cms_point_included_243906370634075863405634576437563978456783457813451