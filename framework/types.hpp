#ifndef _types_included_0823628430807352345098737601245705176456301765716514795
#define _types_included_0823628430807352345098737601245705176456301765716514795

#pragma once

#include <glm/glm.hpp>

template<typename T, typename vec> struct bound
{
    vec vmin, vmax;

    bound() { }
    bound(const vec& vmin, const vec& vmax) : vmin(vmin), vmax(vmax) { }

    bound& include(const bound& in)
    {
        include(in.vmin);
        include(in.vmax);
        return *this;
    }

    bound& include(const vec& in)
    {
        vmin = glm::min(vmin, in);
        vmax = glm::max(vmax, in);
        return *this;
    }

    bound& scale(T scale)
    {
        vmin *= scale;
        vmax *= scale;
        return *this;
    }

    vec center() const
        { return (vmax + vmin) / (T) 2; }

    T width() const
        { return vmax.x - vmin.x; }
    T height() const
        { return vmax.y - vmin.y; }
};

template<typename T, glm::precision P> struct trect : public bound<T, glm::detail::tvec2<T, P>>
{
    typedef glm::detail::tvec2<T, P> vec;
    typedef bound<T, vec> bound_type;

    trect() { }
    trect(const vec& vmin, const vec& vmax) : bound_type(vmin, vmax) { }
    trect(T minx, T miny, T maxx, T maxy) : bound_type(vec(minx, miny), vec(maxx, maxy)) { }

    const vec& getUpperRight() const
        { return bound_type::vmax; }

    const vec& getLowerLeft() const
        { return bound_type::vmin; }

    vec getLowerRight() const
        { return vec(bound_type::vmax.x, bound_type::vmin.y); }
    vec getUpperLeft() const
        { return vec(bound_type::vmin.x, bound_type::vmax.y); }
    T area() const
        { return bound_type::width() * bound_type::height(); }
};

typedef trect<float, glm::precision::highp> rectf;

template<typename T, glm::precision P> struct tbox : public bound<T, glm::detail::tvec3<T, P>>
{
    typedef glm::detail::tvec3<T, P> vec;
    typedef bound<T, vec> bound_type;

    tbox() { }
    tbox(const vec & vmin, const vec & vmax) : bound_type(vmin, vmax) { }
    tbox(T minx, T miny, T minz, T maxx, T maxy, T maxz) : bound_type(vec(minx, miny, minz), vec(maxx, maxy, maxz)) { }

    T depth() const
        { return bound_type::vmax.z - bound_type::vmin.z; }

    T volume() const
        { return bound_type::height() * bound_type::width() * depth(); }
};

typedef tbox<float, glm::precision::highp> boxf;


#endif // _types_included_0823628430807352345098737601245705176456301765716514795
