#ifndef SCENEPARALLAX_HPP_INCLUDED
#define SCENEPARALLAX_HPP_INCLUDED

#include "shader.hpp"
#include "tex2d.hpp"
#include "light.hpp"
#include "trackball_camera.hpp"

struct SceneParallax
{
    SceneParallax(unsigned int screenWidth, unsigned int screenHeight);
    ~SceneParallax();

    std::string getDisplayText (float fps) const;
    
    void mouseMoved(glm::dvec2 mouse_delta, bool shiftPressed);     /* Mouse moved. Movement is supposed to be given in normalized windows coordinates */
    void render();                                                  /* Draws in the current OpenGL context. */

    enum Mode {FLAT = 0, NORMAL = 1, PARALLAX = 2};

    TrackballCamera _camera;
    SpotLight _spotlight;

    int _displayMode;
    float _amplitude;
    unsigned int _nbLayers;
    bool _interpolation;
    bool _selfShadow;
    bool _crop;
    bool _specularMapping;

    GLuint _colorTexture, 
           _normalsTexture,
           _dispTexture,
           _specTexture;

    GLuint vao_id;
    GLuint vbo_id;

    glsl_program_t _simpleShader;
    glsl_program_t _shader;
};

#endif // SCENEPARALLAX_HPP_INCLUDED
