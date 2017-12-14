#pragma once

#include "texture.hpp"
#include "../types/base_object.hpp"

#include <array>
#include <memory>
#include <glm/detail/type_vec3.hpp>

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
    float shininess;
    float shininessStrenght;
    float refractionIndex;
    std::array<std::shared_ptr<Texture2D>, RawTexture::TYPE_MAX> textures;

    Material();
    ~Material();

    void Ambient(const glm::vec3 &val);                                         // material's ambient color component
    void Diffuse(const glm::vec3 &val);                                         // material's diffuse color component
    void Specular(const glm::vec3 &val);                                        // material's specular color component
    void Emissive(const glm::vec3 &val);                                        // material's emissive color component
    void Transparent(const glm::vec3 &val);                                     // material's transparent color component
    void Opacity(const float &val);                                             // material's opacity value
    void Shininess(const float &val);                                           // material's shininess value
    void ShininessStrenght(const float &val);                                   // material's shininess strength value
    void RefractionIndex(const float &val);                                     // material's refractive index value

    const glm::vec3 &Ambient() const;
    const glm::vec3 &Diffuse() const;
    const glm::vec3 &Specular() const;
    const glm::vec3 &Emissive() const;
    const glm::vec3 &Transparent() const;
    float Opacity() const;
    float Shininess() const;
    float ShininessStrenght() const;
    float RefractionIndex() const;

    bool HasTexture(RawTexture::TextureType texType) const;                                         // Determines wheter the material has a texture of the given type
    void AddTexture(const std::shared_ptr<Texture2D> &spTexture, RawTexture::TextureType texType);  // Adds a texture to the material
    bool BindTexture(RawTexture::TextureType texType, bool bindDefault = true) const;               // Binds the material's texture type

};
