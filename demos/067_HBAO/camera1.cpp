
#include <iostream>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "camera1.hpp"

void Camera::move(const glm::vec3 &vec)
{
    glm::mat4 rotmat = glm::yawPitchRoll(glm::radians(orientation.y), glm::radians(orientation.x), glm::radians(orientation.z));
    position += glm::vec3( rotmat * glm::vec4(vec,0.0) );
}

void Camera::draw()
{
    //frustum.drawLines();
    //frustum.drawNormals();
}

void Camera::setup()
{
    // Get window size. It may start out different from the requested size, and will change if the user resizes the window.
    GLFWwindow* window = glfwGetCurrentContext();
    glfwGetWindowSize(window, &screensize.x, &screensize.y );
    if(screensize.y <= 0) screensize.y = 1; // Safeguard against iconified/closed window

    // Set viewport. This is the pixel rectangle we want to draw into.
    glViewport(0, 0, screensize.x, screensize.y); // The entire window

    f32 ratio = (GLfloat)screensize.x / (GLfloat)screensize.y;

    projMat = glm::perspective( fov, ratio, nearfar.x, nearfar.y );

    invProjMat = glm::inverse(projMat);

    if (lookat)
    {
		//gluLookAt(position.x, position.y, position.z, lookat->x, lookat->y, lookat->z, 0.0, 1.0, 0.0);
		viewMat = glm::lookAt(position,*lookat,glm::vec3(0.0,1.0,0.0));
    }
    else
    {
        // Select and setup the modelview matrix.
        //glMatrixMode( GL_MODELVIEW ); // Edit modelview for legacy mode

        viewMat = glm::rotate(mat4(),-orientation.x,glm::vec3(1.0,0.0,0.0));
        viewMat = glm::rotate(viewMat,-orientation.y,glm::vec3(0.0,1.0,0.0));
        viewMat = glm::rotate(viewMat,-orientation.z,glm::vec3(0.0,0.0,1.0));
        viewMat = glm::translate(viewMat,-position);

        //glLoadMatrixf(&viewMat[0][0]);
    }

    invViewMat = glm::inverse(viewMat);

    mat4 m = projMat * viewMat;

    plane[CAMERA_PLANE_LEFT]    = glm::row(m,3) + glm::row(m,0);
    plane[CAMERA_PLANE_RIGHT]   = glm::row(m,3) - glm::row(m,0);
    plane[CAMERA_PLANE_BOTTOM]  = glm::row(m,3) + glm::row(m,1);
    plane[CAMERA_PLANE_TOP]     = glm::row(m,3) - glm::row(m,1);
    plane[CAMERA_PLANE_NEAR]    = glm::row(m,3) + glm::row(m,2);
    plane[CAMERA_PLANE_FAR]     = glm::row(m,3) - glm::row(m,2);

    plane[CAMERA_PLANE_LEFT]    /= glm::length(vec3(plane[CAMERA_PLANE_LEFT]));
    plane[CAMERA_PLANE_RIGHT]   /= glm::length(vec3(plane[CAMERA_PLANE_RIGHT]));
    plane[CAMERA_PLANE_BOTTOM]  /= glm::length(vec3(plane[CAMERA_PLANE_BOTTOM]));
    plane[CAMERA_PLANE_TOP]     /= glm::length(vec3(plane[CAMERA_PLANE_TOP]));
    plane[CAMERA_PLANE_NEAR]    /= glm::length(vec3(plane[CAMERA_PLANE_NEAR]));
    plane[CAMERA_PLANE_FAR]     /= glm::length(vec3(plane[CAMERA_PLANE_FAR]));
}
