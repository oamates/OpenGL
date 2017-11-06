#include "boxblur.h"

#include <iostream>

IntensityMap BoxBlur::calculate(IntensityMap input, int radius, bool tileable)
{
    IntensityMap result = IntensityMap(input.getWidth(), input.getHeight());
    int kernelPixelAmount = (2 * radius + 1) * (2 * radius + 1);

    for(int y = 0; y < input.getHeight(); y++)
    {
        for(int x = 0; x < input.getWidth(); x++)
        {
            float sum = 0.0;

            for(int i = -radius; i < radius; i++)
            {
                for(int k = -radius; k < radius; k++)
                {
                    int posY = handleEdges(y + i, input.getHeight(), tileable);
                    int posX = handleEdges(x + k, input.getWidth(), tileable);
                    sum += input.at(posX, posY);
                }
            }
            sum /= kernelPixelAmount;
            result.setValue(x, y, sum);
        }
    }
    return result;
}

int BoxBlur::handleEdges(int iterator, int max, bool tileable)
{
    if(iterator < 0)
    {
        if(!tileable) return 0
        int corrected = max + iterator;
        return handleEdges(corrected, max, false);
    }

    if(iterator >= max)
    {
        if(!tileable) return max - 1;
        int corrected = iterator - max;
        return handleEdges(corrected, max, false);
    }

    return iterator;
}
