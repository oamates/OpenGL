#ifndef SIMPLE_GRAPHICS_ENGINE_H
#define SIMPLE_GRAPHICS_ENGINE_H

#include <vector>
#include <string>
#include <map>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

struct ShaderManager
{
    static ShaderManager* instance();
    ~ShaderManager();

    void loadShader(std::string name, const char* vs_src, const char* tcs_src, const char* tes_src, const char* gs_src, const char* fs_src);
    GLuint getShader(std::string name);

    std::map<std::string, GLuint> shader_program_IDs;

  private:
    static ShaderManager* instance_;
    ShaderManager();
};
  
struct Object3D
{
    Object3D();
    virtual ~Object3D() {};

    void addChild(Object3D* child);
    void removeChild(Object3D* child);
    virtual void render(glm::mat4 M, GLuint program_ID);

    bool intersects(glm::vec3 point);
    bool intersects(glm::vec3 origin, glm::vec3 direction, float* t);
    glm::mat4 getTotalTransform();

    glm::mat4 transform_matrix_;

    std::vector<Object3D*> children;
    Object3D* parent_;
};

struct AbstractMesh;

struct BoundingBox : public Object3D
{
    BoundingBox(const AbstractMesh* template_mesh);
    BoundingBox(const Object3D);
    BoundingBox();
    ~BoundingBox();
    glm::vec3 getMin(){return min;}
    glm::vec3 getMax(){return max;}
    bool intersects(glm::vec3 point);
    bool intersects(glm::vec3 origin, glm::vec3 direction, float* t);

    glm::vec3 max;
    glm::vec3 min;
};

struct AbstractMesh : public Object3D
{
    AbstractMesh();
    ~AbstractMesh();

    virtual void render(glm::mat4 M, GLuint program_ID) = 0;
    bool intersects(glm::vec3 point);
    bool intersects(glm::vec3 origin, glm::vec3 direction, float* t);

    std::vector<glm::vec3> vertices_;
    virtual void initialize() = 0;

    BoundingBox aabb_;
    
    GLuint vertex_array_ID_;
    GLuint vertex_buffer_;
};

struct TriangleMesh : public AbstractMesh
{
    TriangleMesh();
    TriangleMesh(const char* file_path);
    TriangleMesh(std::vector<glm::vec3> vertices, std::vector<glm::vec3> normals, std::vector<unsigned short> elements);
    ~TriangleMesh();
    virtual void render(glm::mat4 M, GLuint program_ID);
    void initialize();
    std::vector<unsigned short> elements_;
    GLuint element_buffer_;
    GLuint normal_buffer_;
    std::vector<glm::vec3> normals_;
};
  
struct AbstractCamera : public Object3D
{
    AbstractCamera();
    virtual void render(glm::mat4 M, GLuint program_ID) = 0;
    glm::mat4 projection_transform_matrix_;
};

struct PerspectiveCamera : public AbstractCamera
{
    PerspectiveCamera(GLFWwindow* window = nullptr, float fov = 45);
    virtual void render(glm::mat4 M, GLuint program_ID);
    GLFWwindow* window_;
};

struct OrthoCamera : public AbstractCamera
{
    OrthoCamera(GLFWwindow* window = nullptr);
    virtual void render(
    glm::mat4 M,
    GLuint program_ID);
    virtual void render(glm::mat4 M, GLuint program_ID, float left, float right, float bottom, float top, float near, float far);
    GLFWwindow* window_;
};

struct LightSource : public Object3D
{
    LightSource();
    virtual void render(glm::mat4 M, GLuint program_ID);
    
    float intensity;
    glm::vec3 color;
};

struct SimpleGraphicsEngine
{
    SimpleGraphicsEngine();
    virtual ~SimpleGraphicsEngine();
    
    void run();
    virtual void update() = 0;
    virtual void render() = 0;
    
    static GLFWwindow* window_;
    double dt_;
    
    static Object3D* scene_;
    
    bool initialize();
    double time_;
};

struct FBO
{
    FBO(int width, int height, int int_method);
    ~FBO();

    static void useFBO(FBO *out, FBO *in1, FBO *in2);
    static void CHECK_FRAMEBUFFER_STATUS();

    GLuint texid_;
    GLuint fb_;
    GLuint rb_;
    GLuint depth_;
    int width_, height_;
};

#endif