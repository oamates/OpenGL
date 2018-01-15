#ifndef SH_DEFAULT_IMAGE_H
#define SH_DEFAULT_IMAGE_H

#include <memory>

#include "imagebase.hpp"

namespace sh {

struct DefaultImage : public Image 
{
    DefaultImage(int res_x, int res_y);
  
    int res_x() const override;
    int res_y() const override;

    glm::vec3 GetPixel(int x, int y) const override;
    void SetPixel(int x, int y, const glm::vec3& v) override;

    const int _res_x;
    const int _res_y;

    std::unique_ptr<glm::vec3[]> pixels;
};

}  // namespace sh

#endif  // SH_DEFAULT_IMAGE_H
