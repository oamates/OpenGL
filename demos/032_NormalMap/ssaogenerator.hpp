#ifndef _ssaogen_included_37512389471234875230712398696581235687156781260812756
#define _ssaogen_included_37512389471234875230712398696581235687156781260812756

#include "intensitymap.hpp"

struct SsaoGenerator
{
    SsaoGenerator() {};
    QImage calculateSsaomap(QImage normalmap, QImage depthmap, float radius, unsigned int kernelSamples, unsigned int noiseSize);

    std::vector<glm::vec3> generateKernel(unsigned int size);
    std::vector<glm::vec3> generateNoise(unsigned int size);

    double random(double min, double max);
    float lerp(float v0, float v1, float t);
};

#endif // _ssaogen_included_37512389471234875230712398696581235687156781260812756
