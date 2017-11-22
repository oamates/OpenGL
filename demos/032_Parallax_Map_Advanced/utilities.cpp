#include <fstream>
#include <sstream>
#include <ctime>

#include "utilities.hpp"

static int randomInt ()
{
    static bool firstTime = true;

    if (firstTime)
    {
        firstTime = false;
        srand(time(NULL));
    }

    return rand();
}

float randFloat (float minValue, float maxValue)
{
    float range = maxValue - minValue;
    float randomValue = static_cast<float>(randomInt()) / RAND_MAX;
    return minValue + randomValue * range;
}

glm::vec4 randomColor ()
{
    int dominant = randomInt() % 3;
    int missing = randomInt() % 2;
    float varying = randFloat(0.0f, 1.0f);

    return (dominant == 0) ? ((missing == 0) ? glm::vec4(1.0f, 0.0f, varying, 1.0f) : glm::vec4(1.0f, varying, 0.0f, 1.0f)) :
           (dominant == 1) ? ((missing == 0) ? glm::vec4(0.0f, 1.0f, varying, 1.0f) : glm::vec4(varying, 1.0f, 0.0f, 1.0f)) :
                             ((missing == 0) ? glm::vec4(0.0f, varying, 1.0f, 1.0f) : glm::vec4(varying, 0.0f, 1.0f, 1.0f));
}