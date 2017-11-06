#ifndef GAUSSIANBLUR_H
#define GAUSSIANBLUR_H

#include "intensitymap.hpp"

struct GaussianBlur
{
    GaussianBlur() {};
    IntensityMap calculate(IntensityMap& input, double radius, bool tileable);

    std::vector<double> boxesForGauss(double sigma, int n);
    void gaussBlur(IntensityMap &input, IntensityMap &result, double radius, bool tileable);
    void boxBlur(IntensityMap &input, IntensityMap &result, double radius, bool tileable);
    void boxBlurH(IntensityMap &input, IntensityMap &result, double radius, bool tileable);
    void boxBlurT(IntensityMap &input, IntensityMap &result, double radius, bool tileable);
    int handleEdges(int iterator, int max, bool tileable) const;
};

#endif // GAUSSIANBLUR_H
