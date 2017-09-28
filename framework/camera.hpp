#ifndef _camera3d_included_1094736872433454589780542615436578409788414354799951
#define _camera3d_included_1094736872433454589780542615436578409788414354799951

#include <glm/glm.hpp>

//=======================================================================================================================================================================================================================
// Euclidean space camera
//=======================================================================================================================================================================================================================

struct camera_t
{
    double linear_speed;
    double angular_speed;

    glm::mat4 view_matrix;
    glm::mat4 projection_matrix;

    camera_t(const double linear_speed = 2.0, const double angular_speed = 0.125, const glm::mat4& view_matrix = glm::mat4(1.0f))
        : linear_speed(linear_speed), angular_speed(angular_speed), view_matrix(view_matrix) {};

    void translate(const glm::vec3& shift);
    void move_forward(double dt);
    void move_backward(double dt);
    void straight_right(double dt);
    void straight_left(double dt);
    void rotateXY(const glm::dvec2& direction, double dt);

    void infinite_perspective(float view_angle, float aspect_ratio, float znear);

    glm::mat4 projection_view_matrix();
    glm::mat4 camera_matrix();
    glm::vec3 position();
    glm::vec2 focal_scale();

};

//=======================================================================================================================================================================================================================
// Hyperbolic 3-space moving camera
//=======================================================================================================================================================================================================================

struct hyperbolic_camera_t
{
    double linear_speed;
    double angular_speed;

    glm::dmat4 view_matrix;
    glm::mat4 projection_matrix;

    hyperbolic_camera_t(const double linear_speed = 2.0, const double angular_speed = 0.125, const glm::mat4& view_matrix = glm::mat4(1.0f))
        : linear_speed(linear_speed), angular_speed(angular_speed), view_matrix(view_matrix) {};

    void translate(const glm::dvec3& shift);
    void move_forward(double distance);
    void move_backward(double distance);
    void straight_right(double distance);
    void straight_left(double distance);

    void rotateXY(const glm::dvec2& direction, double dt);

    void infinite_perspective(float view_angle, float aspect_ratio, float znear);
    glm::mat4 projection_view_matrix();
};

#endif  // _camera3d_included_1094736872433454589780542615436578409788414354799951




