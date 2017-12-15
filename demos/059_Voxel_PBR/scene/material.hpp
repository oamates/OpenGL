#pragma once

#include <random>
#include <array>
#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/detail/type_vec3.hpp>
#include <glm/detail/func_common.hpp>

#include "texture.hpp"
#include "../types/base_object.hpp"

// Contains parameters that describe a material properties. It is usually bound to a Mesh for rendering.
struct Material : public BaseObject
{
    enum ShadingMode
    {
        Flat,
        Gourad,
        Phong,
        Blinn,
        Toon,
        OrenNayar,
        Minnaert,
        CookTorrance,
        NoShading,
        Fresnel
    };
    enum BlendMode
    {
        Default,
        Additive
    };

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 emissive;
    glm::vec3 transparent;
    float opacity;
    float shininess_exponent;
    float shininess_strength;
    float refraction_index;
    std::array<std::shared_ptr<Texture2D>, RawTexture::TYPE_MAX> textures;

    Material()
        : opacity(1.0f), shininess_exponent(0.5f), shininess_strength(1.0f), refraction_index(1.5f)
    {
        name = "Default Material";
        ambient = glm::vec3(0.25f);
        diffuse = glm::vec3(0.75f);
        specular = glm::vec3(0.2f);
        emissive = transparent = glm::vec3(0.0f);
    }

    ~Material() {}

    bool HasTexture(RawTexture::TextureType texType) const                                          // Determines wheter the material has a texture of the given type
        { return textures[texType] != 0; }

    void AddTexture(const std::shared_ptr<Texture2D> &spTexture, RawTexture::TextureType texType)   // Adds a texture to the material
    {
        textures[texType] = spTexture;
        if (texType == RawTexture::Specular)                                                        // will set the material specular to one so multiplying keeps the texture values
            specular = glm::vec3(1.0f);
    }

    bool BindTexture(RawTexture::TextureType texType, bool bindDefault = true) const                // Binds the material's texture type
    {
        if (textures[texType] != 0)
        {
            textures[texType]->Bind();
            return true;
        }

        if (bindDefault)
            Texture2D::GetDefaultTexture()->Bind();

        return false;
    }
};
