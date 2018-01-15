#include "default_image.hpp"

namespace sh {

DefaultImage::DefaultImage(int res_x, int res_y) : _res_x(res_x), _res_y(res_y)
{
    pixels.reset(new glm::vec3[_res_x * _res_y]);
}

int DefaultImage::res_x() const
    { return _res_x; }

int DefaultImage::res_y() const
    { return _res_y; }

glm::vec3 DefaultImage::GetPixel(int x, int y) const
{
    int index = x + y * _res_x;
    return pixels[index];
}

void DefaultImage::SetPixel(int x, int y, const glm::vec3& v)
{
    int index = x + y * _res_x;
    pixels[index] = v;
}

}  // namespace sh
