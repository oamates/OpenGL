#include "intensitymap.hpp"
#include <QColor>

IntensityMap::IntensityMap(int width, int height)
{
    map = std::vector< std::vector<double> >(height, std::vector<double>(width, 0.0));
}

IntensityMap::IntensityMap(const QImage& rgbImage, Mode mode, bool useRed, bool useGreen, bool useBlue, bool useAlpha)
{
    map = std::vector<std::vector<double>>(rgbImage.height(), std::vector<double>(rgbImage.width(), 0.0));

    for(int y = 0; y < rgbImage.height(); y++)
    {
        for(int x = 0; x < rgbImage.width(); x++)
        {
            double intensity = 0.0;

            const double r = QColor(rgbImage.pixel(x, y)).redF();
            const double g = QColor(rgbImage.pixel(x, y)).greenF();
            const double b = QColor(rgbImage.pixel(x, y)).blueF();
            const double a = QColor(rgbImage.pixel(x, y)).alphaF();

            if(mode == AVERAGE)
            {
                //take the average out of all selected channels
                int num_channels = 0;

                if(useRed) {
                    intensity += r;
                    num_channels++;
                }
                if(useGreen) {
                    intensity += g;
                    num_channels++;
                }
                if(useBlue) {
                    intensity += b;
                    num_channels++;
                }
                if(useAlpha) {
                    intensity += a;
                    num_channels++;
                }

                if(num_channels != 0)
                    intensity /= num_channels;
                else
                    intensity = 0.0;
            }
            else if(mode == MAX)
            {
                const double tempR = useRed ? r : 0.0;
                const double tempG = useGreen ? g : 0.0;
                const double tempB = useBlue ? b : 0.0;
                const double tempA = useAlpha ? a : 0.0;
                const double tempMaxRG = std::max(tempR, tempG);
                const double tempMaxBA = std::max(tempB, tempA);
                intensity = std::max(tempMaxRG, tempMaxBA);
            }

            this->map.at(y).at(x) = intensity;
        }
    }
}

double IntensityMap::at(int x, int y) const
{
    return this->map.at(y).at(x);
}

double IntensityMap::at(int pos) const
{
    const int x = pos % this->getWidth();
    const int y = pos / this->getWidth();
    return this->at(x, y);
}

void IntensityMap::setValue(int x, int y, double value)
{
    this->map.at(y).at(x) = value;
}

void IntensityMap::setValue(int pos, double value)
{
    const int x = pos % this->getWidth();
    const int y = pos / this->getWidth();

    this->map.at(y).at(x) = value;
}

size_t IntensityMap::getWidth() const
{
    return this->map.at(0).size();
}

size_t IntensityMap::getHeight() const
{
    return this->map.size();
}

void IntensityMap::invert()
{
    for(int y = 0; y < this->getHeight(); y++)
    {
        for(int x = 0; x < this->getWidth(); x++)
        {
            const double inverted = 1.0 - this->map.at(y).at(x);
            this->map.at(y).at(x) = inverted;
        }
    }
}

QImage IntensityMap::convertToQImage() const
{
    QImage result(this->getWidth(), this->getHeight(), QImage::Format_ARGB32);
    for(int y = 0; y < this->getHeight(); y++)
    {
        QRgb *scanline = (QRgb*) result.scanLine(y);
        for(int x = 0; x < this->getWidth(); x++)
        {
            const int c = 255 * map.at(y).at(x);
            scanline[x] = qRgba(c, c, c, 255);
        }
    }
    return result;
}
