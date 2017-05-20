#define GLEW_STATIC
#include <GL/glew.h> 
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "camera1.hpp"

void Camera::move(const glm::vec3 &vec)
{
    glm::mat4 rotmat = glm::yawPitchRoll(glm::radians(orientation.y), glm::radians(orientation.x), glm::radians(orientation.z));
    position += glm::vec3(rotmat * glm::vec4(vec, 0.0));
}

void Camera::draw()
{
}

void Camera::setup()
{
    GLFWwindow* window = glfwGetCurrentContext();
    glfwGetWindowSize(window, &screensize.x, &screensize.y);
    if(screensize.y <= 0) screensize.y = 1;

    glViewport(0, 0, screensize.x, screensize.y);

    float ratio = (float)screensize.x / screensize.y;

    projMat = glm::perspective(fov, ratio, nearfar.x, nearfar.y);

    invProjMat = glm::inverse(projMat);

    if (lookat)
		viewMat = glm::lookAt(position,*lookat,glm::vec3(0.0,1.0,0.0));
    else
    {
        viewMat = glm::rotate(glm::mat4(1.0f), -orientation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        viewMat = glm::rotate(viewMat, -orientation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        viewMat = glm::rotate(viewMat, -orientation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        viewMat = glm::translate(viewMat, -position);
    }

    invViewMat = glm::inverse(viewMat);

    glm::mat4 m = projMat * viewMat;

    plane[CAMERA_PLANE_LEFT]    = glm::row(m, 3) + glm::row(m, 0);
    plane[CAMERA_PLANE_RIGHT]   = glm::row(m, 3) - glm::row(m, 0);
    plane[CAMERA_PLANE_BOTTOM]  = glm::row(m, 3) + glm::row(m, 1);
    plane[CAMERA_PLANE_TOP]     = glm::row(m, 3) - glm::row(m, 1);
    plane[CAMERA_PLANE_NEAR]    = glm::row(m, 3) + glm::row(m, 2);
    plane[CAMERA_PLANE_FAR]     = glm::row(m, 3) - glm::row(m, 2);

    plane[CAMERA_PLANE_LEFT]    /= glm::length(glm::vec3(plane[CAMERA_PLANE_LEFT]));
    plane[CAMERA_PLANE_RIGHT]   /= glm::length(glm::vec3(plane[CAMERA_PLANE_RIGHT]));
    plane[CAMERA_PLANE_BOTTOM]  /= glm::length(glm::vec3(plane[CAMERA_PLANE_BOTTOM]));
    plane[CAMERA_PLANE_TOP]     /= glm::length(glm::vec3(plane[CAMERA_PLANE_TOP]));
    plane[CAMERA_PLANE_NEAR]    /= glm::length(glm::vec3(plane[CAMERA_PLANE_NEAR]));
    plane[CAMERA_PLANE_FAR]     /= glm::length(glm::vec3(plane[CAMERA_PLANE_FAR]));
}
