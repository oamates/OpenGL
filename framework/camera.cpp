//=======================================================================================================================================================================================================================
// Euclidean and hyperbolic 3d camera structure implementation
//=======================================================================================================================================================================================================================

#include <glm/gtx/transform.hpp> 
#include <glm/gtc/matrix_transform.hpp>     
#include "camera.hpp"

//=======================================================================================================================================================================================================================
// Let camera matrix be the matrix which transforms world space origin to the position of the camera and world frame to standard frame rigidly attached to the camera.
// Then view matrix is exactly inverse of this matrix. Therefore if camera position or orientation changes and the change is given in the camera own coordinate system
//       new camera_matrix = old camera_matrix * local_transform_matrix
// then for view matrices the relation is :
//       new view_matrix = inverse(local_transform_matrix) * old view_matrix
//=======================================================================================================================================================================================================================

//=======================================================================================================================================================================================================================
// generic camera position shift
//=======================================================================================================================================================================================================================
void camera_t::translate(const glm::vec3& shift)
    { view_matrix[3] -= glm::vec4(shift, 0.0f); }

//=======================================================================================================================================================================================================================
// special cases : forward/backward translation. The sign is + since camera z axis points backward (index of z-axis = 2)
//=======================================================================================================================================================================================================================
void camera_t::move_forward(double dt)
    { view_matrix[3][2] += linear_speed * dt; }

void camera_t::move_backward(double dt)
    { view_matrix[3][2] -= linear_speed * dt; }

//=======================================================================================================================================================================================================================
// special cases : right/left translation. Right direction is the axis x (index = 0)
//=======================================================================================================================================================================================================================
void camera_t::straight_right(double dt)
    { view_matrix[3][0] -= linear_speed * dt; }

void camera_t::straight_left(double dt)
    { view_matrix[3][0] += linear_speed * dt; }

//=======================================================================================================================================================================================================================
// camera rotation about the axis lying in XY - plane and orthogonal(!) to (dx, dy) mouse displacement
//=======================================================================================================================================================================================================================
void camera_t::rotateXY(const glm::dvec2& direction, double dt)
{
    double theta = angular_speed * dt;
    double dx = -direction.x;
    double dy = direction.y;
    double cs = glm::cos(theta);
    double sn = glm::sin(theta);
    double _1mcs = 1.0 - cs;
    float sndx = sn * dx;
    float sndy = sn * dy;
    float _1mcsdx = _1mcs * dx;
    float _1mcsdy = _1mcs * dy;
    glm::mat4 rotation_matrix = glm::mat4 (1.0f - dx * _1mcsdx,      - dy * _1mcsdx,  sndx, 0.0f, 
                                                - dx * _1mcsdy, 1.0f - dy * _1mcsdy,  sndy, 0.0f,
                                                        - sndx,              - sndy,    cs, 0.0f,
                                                          0.0f,                0.0f,  0.0f, 1.0f);
    view_matrix = rotation_matrix * view_matrix;
}

void camera_t::infinite_perspective(float view_angle, float aspect_ratio, float znear)
    { projection_matrix = glm::infinitePerspective (view_angle, aspect_ratio, znear); };

glm::mat4 camera_t::projection_view_matrix()
    { return projection_matrix * view_matrix; }

glm::mat4 camera_t::camera_matrix()
    { return glm::inverse(view_matrix); }

glm::vec3 camera_t::position()
    { return -glm::inverse(glm::mat3(view_matrix)) * glm::vec3(view_matrix[3]); }


//=======================================================================================================================================================================================================================
// For hyperboloid model of hyperbolic space motions are linear transformations preserving Lorentz metric dx * dx + dy * dy + dz * dz - dw * dw.
// View matrix does not split however, i.e translation is not separable from rotation (m[0][3], m[0][1], m[0][2] are not neccessary = 0)
// Everything else is similar - view matrix is updated by left multiplication by inverse of local transform matrix
// It is stored as a matrix of doubles to tame down numerical instabilities.
//=======================================================================================================================================================================================================================

void hyperbolic_camera_t::translate(const glm::dvec3& shift) {}

//=======================================================================================================================================================================================================================
// hyperbolic translation along Z is equivalent to ZW - Lorentz rotation
// | 1  0       0           0      |
// | 0  1       0           0      |                        
// | 0  0  cosh(theta) sinh(theta) |
// | 0  0  sinh(theta) cosh(theta) |
//=======================================================================================================================================================================================================================
void hyperbolic_camera_t::move_forward(double distance)
{
    double sh = sinh(distance);
    double ch = cosh(distance);

    double m02 = view_matrix[0].z * ch + view_matrix[0].w * sh; 
    double m03 = view_matrix[0].z * sh + view_matrix[0].w * ch; 
    double m12 = view_matrix[1].z * ch + view_matrix[1].w * sh; 
    double m13 = view_matrix[1].z * sh + view_matrix[1].w * ch; 
    double m22 = view_matrix[2].z * ch + view_matrix[2].w * sh; 
    double m23 = view_matrix[2].z * sh + view_matrix[2].w * ch;
    double m32 = view_matrix[3].z * ch + view_matrix[3].w * sh; 
    double m33 = view_matrix[3].z * sh + view_matrix[3].w * ch;

    view_matrix[0].z = m02; 
    view_matrix[0].w = m03; 
    view_matrix[1].z = m12; 
    view_matrix[1].w = m13; 
    view_matrix[2].z = m22; 
    view_matrix[2].w = m23;
    view_matrix[3].z = m32; 
    view_matrix[3].w = m33;
}

void hyperbolic_camera_t::move_backward(double distance) 
    { move_forward(-distance); }

//=======================================================================================================================================================================================================================
// hyperbolic translation along X is equivalent to XW - Lorentz rotation
// | cosh(theta) 0 0 sinh(theta) |
// |      0      1 0      0      |
// |      0      0 1      0      |
// | sinh(theta) 0 0 cosh(theta) |
//=======================================================================================================================================================================================================================
void hyperbolic_camera_t::straight_left(double distance)
{
    double sh = sinh(distance);
    double ch = cosh(distance);

    double m00 = view_matrix[0][0] * ch + view_matrix[0][3] * sh; 
    double m03 = view_matrix[0][0] * sh + view_matrix[0][3] * ch; 
    double m10 = view_matrix[1][0] * ch + view_matrix[1][3] * sh; 
    double m13 = view_matrix[1][0] * sh + view_matrix[1][3] * ch; 
    double m20 = view_matrix[2][0] * ch + view_matrix[2][3] * sh; 
    double m23 = view_matrix[2][0] * sh + view_matrix[2][3] * ch;
    double m30 = view_matrix[3][0] * ch + view_matrix[3][3] * sh; 
    double m33 = view_matrix[3][0] * sh + view_matrix[3][3] * ch; 

    view_matrix[0][0] = m00; 
    view_matrix[0][3] = m03; 
    view_matrix[1][0] = m10; 
    view_matrix[1][3] = m13; 
    view_matrix[2][0] = m20; 
    view_matrix[2][3] = m23;
    view_matrix[3][0] = m30; 
    view_matrix[3][3] = m33;    
}

void hyperbolic_camera_t::straight_right(double distance)
    { straight_left(-distance); }


void hyperbolic_camera_t::rotateXY(const glm::dvec2& direction, double dt)
{
    double theta = angular_speed * dt;
    double dx = -direction.x;
    double dy = direction.y;
    double cs = glm::cos(theta);
    double sn = glm::sin(theta);
    double _1mcs = 1.0 - cs;
    double sndx = sn * dx;
    double sndy = sn * dy;
    double _1mcsdx = _1mcs * dx;
    double _1mcsdy = _1mcs * dy;
    glm::dmat4 rotation_matrix = glm::mat4 (1.0 - dx * _1mcsdx,     - dy * _1mcsdx,  sndx, 0.0, 
                                                - dx * _1mcsdy, 1.0 - dy * _1mcsdy,  sndy, 0.0,
                                                        - sndx,             - sndy,    cs, 0.0,
                                                           0.0,                0.0,   0.0, 1.0);
    view_matrix = rotation_matrix * view_matrix;
}

void hyperbolic_camera_t::infinite_perspective(float view_angle, float aspect_ratio, float znear)
    { projection_matrix = glm::infinitePerspective (view_angle, aspect_ratio, znear); }

glm::mat4 hyperbolic_camera_t::projection_view_matrix()
    { return projection_matrix * glm::mat4(view_matrix); }