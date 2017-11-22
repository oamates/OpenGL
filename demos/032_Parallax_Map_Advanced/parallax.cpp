#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>

#include "gl_aux.hpp"
#include "parallax.hpp"
#include "image.hpp"

SceneParallax::SceneParallax(unsigned int screenWidth, unsigned int screenHeight)
    : _camera(glm::vec3(0.5f, 0.5f, 0.0f), 1),
      _displayMode(PARALLAX),
      _amplitude(0.03f),
      _nbLayers(10),
      _interpolation(true),
      _selfShadow(true),
      _crop(false),
      _specularMapping(true),
      vbo_id(-1),
      vao_id(-1),
      _simpleShader(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/simple.vs"),
                    glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/simple.fs")),
      _shader(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/parallax.vs"),
              glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/parallax.fs"))
{
    _camera.setAspectRatio(screenWidth, screenHeight);
    _camera.setNearDist(0.01f);
    _camera.setFarDist(20.0f);
    _spotlight.setFocusPoint(glm::vec3(0.5f, 0.5f, 0.0f));
    _spotlight.setLongitude(3.0f);

    glActiveTexture(GL_TEXTURE0);
    _colorTexture   = image::png::texture2d("res/color.png");
    glActiveTexture(GL_TEXTURE1);
    _normalsTexture = image::png::texture2d("res/normals.png");
    glActiveTexture(GL_TEXTURE2);
    _dispTexture    = image::png::texture2d("res/displacement.png");
    glActiveTexture(GL_TEXTURE3);
    _specTexture    = image::png::texture2d("res/specular.png");

    glm::vec2 vertices[] = 
    {
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 1.0f),
        glm::vec2(0.0f, 1.0f),
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 1.0f),
        glm::vec2(1.0f, 0.0f)
    };

    for (int v = 0; v < 6; ++v)
        vertices[v] = 4.0f * (vertices[v] - glm::vec2(0.5f, 0.5f));

    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);

    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), glm::value_ptr(vertices[0]), GL_STATIC_DRAW);
}

SceneParallax::~SceneParallax()
{
    if (vbo_id != -1) glDeleteBuffers(1, &vbo_id);
    if (vao_id != -1) glDeleteVertexArrays(1, &vao_id);
}

std::string SceneParallax::getDisplayText (float fps) const
{
    std::stringstream s;

    s << "fps: " << static_cast<int>(fps) << std::endl << std::endl;

    std::string mode = "FLAT";
    if (_displayMode == NORMAL)
        mode = "NORMAL";
    else if (_displayMode == PARALLAX)
        mode = "PARALLAX";
    s << "display mode (D): " << mode << std::endl << std::endl;

    s << "amplitude (A/Z): " << std::setprecision(3) << _amplitude << std::endl;
    s << "layers (L/M): " << _nbLayers << std::endl;
    s << "interpolation (I): " << _interpolation << std::endl;
    s << "self-shadowing (S): " << _selfShadow << std::endl;
    s << "cropping (C): " << _crop << std::endl;
    s << "specular mapping (V): " << _specularMapping;

    return s.str();
}

void SceneParallax::mouseMoved(glm::dvec2 mouse_delta, bool shiftPressed)
{
    TrackballObject* trackball = nullptr;
    float invertedAxis;
    if (shiftPressed)
    {
        trackball = &_spotlight;
        invertedAxis = 1.0f;
    }
    else
    {
        trackball = &_camera;
        invertedAxis = -1.0f;
    }

    float latitude = trackball->getLatitude();
    float longitude = trackball->getLongitude();

    longitude += invertedAxis * 2.0f * M_PI * mouse_delta.x;
    latitude += invertedAxis * 0.5f * M_PI * mouse_delta.y;

    trackball->setLatitude(latitude);
    trackball->setLongitude(longitude);
}

void SceneParallax::render ()
{
    glClearColor(0.8, 0.8, 0.8, 1.0);

    _shader.enable();

    _shader["colorTexture"]    = 0;
    _shader["normalsTexture"]  = 1;
    _shader["dispTexture"]     = 2;
    _shader["specularTexture"] = 3;

    _shader["amplitude"]       = _amplitude;

    _shader["mode"]            = (int) _displayMode;
    _shader["nbLayers"]        = (int) _nbLayers;
    _shader["interpolation"]   = (int) _interpolation;
    _shader["selfShadow"]      = (int) _selfShadow;
    _shader["crop"]            = (int) _crop;
    _shader["specularMapping"] = (int) _specularMapping;

    _camera.sendToShader(_shader);
    _spotlight.sendToShader(_shader);

    glBindVertexArray(vao_id);
    glDisable(GL_CULL_FACE);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    _simpleShader.enable();
    _camera.sendToShader(_simpleShader);
    _spotlight.draw(_simpleShader, _camera);


    glUseProgram(0);
}
