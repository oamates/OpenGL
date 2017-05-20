#ifndef CAMERA_H
#define CAMERA_H

#define CAMERA_PLANE_LEFT   0
#define CAMERA_PLANE_RIGHT  1
#define CAMERA_PLANE_BOTTOM 2
#define CAMERA_PLANE_TOP    3
#define CAMERA_PLANE_NEAR   4
#define CAMERA_PLANE_FAR    5

struct Camera
{
    glm::vec2 nearfar;
    glm::ivec2 screensize;
    float fov;

    glm::vec3 position;
    glm::vec3 orientation;
    glm::mat4 viewMat, projMat;
    glm::mat4 invViewMat, invProjMat;
    glm::vec4 plane[6];
    glm::vec3 *lookat;

    Camera()
        : nearfar(glm::vec2(0.1f, 100.0f)), screensize(glm::ivec2(1024, 768)), fov(glm::radians(60.0f)), lookat(0)
        {}

    void setup();
    void draw();

    void setPosition(const glm::vec3& pos)
        { position = pos; }
    void setPosition(float x, float y, float z)
        { position = glm::vec3(x,y,z);}
    void setOrientation(const glm::vec3& ori) 
        { orientation = ori; }
    void setOrientation(float x, float y, float z) 
        { orientation = glm::vec3(x, y, z); }

    void lookAt(glm::vec3 *pos)
        { lookat = pos; }
    void move(const glm::vec3& vec);
    void translate(const glm::vec3& vec)
        { position += vec; }

    const glm::vec3 &getPosition()
        { return position; }
    const glm::vec3 &getOrientation() 
        { return orientation; }

    float getFov()
        { return fov; }
    float getNear()
        { return nearfar.x; }
    float getFar()
        { return nearfar.y; }

    const glm::mat4 &getProjMatrix()
        { return projMat; }
    const glm::mat4 &getViewMatrix()
        { return viewMat; }
    const glm::mat4 &getInverseViewMatrix() 
        { return invViewMat; }
    const glm::mat4 &getInverseProjMatrix()
        { return invProjMat; }
};

#endif
