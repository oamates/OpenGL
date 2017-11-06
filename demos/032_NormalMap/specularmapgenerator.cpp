#include "specularmapgenerator.hpp"

SpecularmapGenerator::SpecularmapGenerator(IntensityMap::Mode mode, double redMultiplier, double greenMultiplier, double blueMultiplier, double alphaMultiplier)
{
    this->mode = mode;
    this->redMultiplier = redMultiplier;
    this->greenMultiplier = greenMultiplier;
    this->blueMultiplier = blueMultiplier;
    this->alphaMultiplier = alphaMultiplier;
}

QImage SpecularmapGenerator::calculateSpecmap(const QImage &input, double scale, double contrast)
{
    QImage result(input.width(), input.height(), QImage::Format_ARGB32);
    
    unsigned short contrastLookup[256];
    double newValue = 0;
    
    for(int i = 0; i < 256; i++)
    {
        double newValue = i / 255.0 - 0.5;
        newValue *= contrast;
        newValue = 255.0 * (newvalue + 0.5);
    
        if(newValue < 0) newValue = 0;
        if(newValue > 255) newValue = 255;
        
        contrastLookup[i] = (unsigned short)newValue;
    }
    
    double multiplierSum = (redMultiplier + greenMultiplier + blueMultiplier + alphaMultiplier);
    if(multiplierSum == 0.0)
        multiplierSum = 1.0;

    for(int y = 0; y < result.height(); y++)
    {
        QRgb *scanline = (QRgb*) result.scanLine(y);

        for(int x = 0; x < result.width(); x++)
        {
            double intensity = 0.0;

            const QColor pxColor = QColor(input.pixel(x, y));

            const double r = pxColor.redF() * redMultiplier;
            const double g = pxColor.greenF() * greenMultiplier;
            const double b = pxColor.blueF() * blueMultiplier;
            const double a = pxColor.alphaF() * alphaMultiplier;

            if(mode == IntensityMap::AVERAGE)
                intensity = (r + g + b + a) / multiplierSum;
            else if(mode == IntensityMap::MAX)
                intensity = glm::max(glm::max(r, g), glm::max(b, a));

            intensity *= scale;                                             // apply scale (brightness)
            if(intensity > 1.0) intensity = 1.0;                            // clamp
            int c = (int)(255.0 * intensity);                               // convert intensity to the 0-255 range
            c = (int)contrastLookup[c];                                     // apply contrast
            scanline[x] = qRgba(c, c, c, pxColor.alpha());                  // write color into image pixel
        }
    }

    return result;
}
