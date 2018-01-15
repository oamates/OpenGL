#ifndef SH_IMAGE_H
#define SH_IMAGE_H

#include <glm/glm.hpp>

namespace sh {

struct Image 
{
    Image() {}
    virtual ~Image() {}

    virtual int res_x() const = 0;
    virtual int res_y() const = 0;

    virtual glm::vec3 GetPixel(int x, int y) const = 0;
    virtual void SetPixel(int x, int y, const glm::vec3& v) = 0;
};

}  // namespace sh

#endif  // IMAGE_H