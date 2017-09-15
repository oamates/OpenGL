#ifndef _matrix_transform_included_11235861384756120375120384512803745123845245
#define _matrix_transform_included_11235861384756120375120384512803745123845245

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtx/transform.hpp>

//=======================================================================================================================================================================================================================
// Perspective matrix calculation routines
//=======================================================================================================================================================================================================================
template<typename real_t> glm::tmat4x4<real_t> perspective(const ovrFovPort& fov, real_t zNear, real_t zFar)
{
    real_t x_scale  = 2.0f / (fov.LeftTan + fov.RightTan);
    real_t x_offset = 0.5f * (fov.LeftTan - fov.RightTan) * x_scale;
    real_t y_scale  = 2.0f / (fov.UpTan + fov.DownTan);
    real_t y_offset = 0.5f * (fov.UpTan - fov.DownTan) * y_scale;
    real_t dz = zNear - zFar;

    return glm::tmat4x4<real_t>(
                glm::tvec4<real_t>(     x_scale, (real_t) 0.0,                         (real_t) 0.0, (real_t)  0.0),
                glm::tvec4<real_t>((real_t) 0.0,      y_scale,                         (real_t) 0.0, (real_t)  0.0),
                glm::tvec4<real_t>(   -x_offset,     y_offset,                  (zNear + zFar) / dz, (real_t) -1.0),
                glm::tvec4<real_t>((real_t) 0.0, (real_t) 0.0, ((real_t) 2.0) * (zFar * zNear) / dz, (real_t)  0.0)
           ); 
}

template<typename real_t> glm::tmat4x4<real_t> infinite_perspective(const ovrFovPort& fov, real_t zNear)
{
    real_t x_scale  = 2.0 / (fov.LeftTan + fov.RightTan);
    real_t x_offset = 0.5 * (fov.LeftTan - fov.RightTan) * x_scale;
    real_t y_scale  = 2.0 / (fov.UpTan + fov.DownTan);
    real_t y_offset = 0.5 * (fov.UpTan - fov.DownTan) * y_scale;
                                                                                     
    return glm::tmat4x4<real_t>(
                glm::tvec4<real_t>(     x_scale, (real_t) 0.0,            (real_t) 0.0,  (real_t) 0.0),
                glm::tvec4<real_t>((real_t) 0.0,      y_scale,            (real_t) 0.0,  (real_t) 0.0),
                glm::tvec4<real_t>(   -x_offset,     y_offset,           (real_t) -1.0, (real_t) -1.0),
                glm::tvec4<real_t>((real_t) 0.0, (real_t) 0.0, ((real_t) -2.0) * zNear,  (real_t) 0.0)
           );
}

#endif // _matrix_transform_included_11235861384756120375120384512803745123845245