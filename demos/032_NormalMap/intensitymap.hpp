#ifndef _intensity_map_included_21387569823597682375237846512756820730162378121
#define _intensity_map_included_21387569823597682375237846512756820730162378121

struct IntensityMap
{
    enum Mode
    {
        AVERAGE,
        MAX
    };

    IntensityMap() {};
    IntensityMap(int width, int height);
    IntensityMap(const QImage &rgbImage, Mode mode, bool useRed = true, bool useGreen = true, bool useBlue = true, bool useAlpha = false);
    double at(int x, int y) const;
    double at(int pos) const;
    void setValue(int x, int y, double value);
    void setValue(int pos, double value);
    size_t getWidth() const;
    size_t getHeight() const;
    void invert();
    QImage convertToQImage() const;

    std::vector<std::vector<double>> map;
};

#endif // _intensity_map_included_21387569823597682375237846512756820730162378121
