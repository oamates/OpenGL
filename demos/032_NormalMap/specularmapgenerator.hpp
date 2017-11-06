#ifndef _specular_map_included_152389063890756120389561892037561820375689123675
#define _specular_map_included_152389063890756120389561892037561820375689123675

#include "intensitymap.hpp"

struct SpecularmapGenerator
{
    double redMultiplier, greenMultiplier, blueMultiplier, alphaMultiplier;
    IntensityMap::Mode mode;

    SpecularmapGenerator(IntensityMap::Mode mode, double redMultiplier, double greenMultiplier, double blueMultiplier, double alphaMultiplier);
    QImage calculateSpecmap(const QImage& input, double scale, double contrast);
};

#endif // _specular_map_included_152389063890756120389561892037561820375689123675
