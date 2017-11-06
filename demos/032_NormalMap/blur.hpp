#ifndef BOXBLUR_H
#define BOXBLUR_H

#include "intensitymap.hpp"

struct BoxBlur
{
    BoxBlur() {};
    IntensityMap calculate(IntensityMap input, int radius, bool tileable);

    int handleEdges(int iterator, int max, bool tileable);
};

#endif // BOXBLUR_H
