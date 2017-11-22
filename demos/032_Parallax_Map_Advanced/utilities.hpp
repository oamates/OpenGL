#ifndef UTILITIES_HPP_INCLUDED
#define UTILITIES_HPP_INCLUDED

#include <sstream>
#include <string>
#include <algorithm>
#include <cstdlib>

#include <glm/glm.hpp>

float randFloat (float minValue = 0.0f, float maxValue = 1.0f);

glm::vec4 randomColor();

template<typename T> inline T clamp (T const& lowest, T const& heighest, T const& value)
{
    return std::min(heighest, std::max(lowest, value));
}

template<typename T> std::string toString (T const& object)
{
    std::stringstream s;
    s << object;
    return s.str();
}
#endif // UTILITIES_HPP_INCLUDED
