#pragma once

#include <set>
#include "../types/base_object.hpp"

#include <memory>
#include <glm/detail/type_vec1.hpp>
#include <glm/detail/type_vec4.hpp>
#include <oglplus/texture.hpp>

struct RawTexture : public BaseObject
{
    std::string filepath;
    unsigned int height;
    unsigned int width;
    unsigned int depth;
    unsigned int lineWidth;
    unsigned int bitsPerPixel;
    std::unique_ptr<unsigned char> rawData;

    enum TextureType
    {
        None,
        Diffuse,
        Specular,
        Ambient,
        Emissive,
        Height,
        Normals,
        Shininess,
        Opacity,
        Displacement,
        Lightmap,
        Reflection,
        Unknow,
        TYPE_MAX
    };
        
    bool IsType(TextureType type);                                  // Determines the texture is of the given type.
    std::string GetFilepath() const;                                // Gets the texture filepath.
    std::set<TextureType> textureTypes;

    RawTexture();
    ~RawTexture();
    RawTexture(RawTexture const &) = delete;
    RawTexture &operator=(RawTexture const &) = delete;
};

struct Texture2D : public RawTexture
{
    std::unique_ptr<oglplus::Texture> oglTexture;

    Texture2D() {};
    ~Texture2D() {};
    Texture2D(Texture2D const &) = delete;
    Texture2D &operator = (Texture2D const &) = delete;
        
    void Load(oglplus::TextureMinFilter minFilter,                      // Loads the associated texture file with the given parameters.
              oglplus::TextureMagFilter magFilter,
              oglplus::TextureWrap wrapS,
              oglplus::TextureWrap wrapT,
              bool generateMipmaps = true,
              glm::vec4 borderColor = glm::vec4(0.f));
        
    void Bind() const;                                                  // Binds the texture.
    int Name() const;                                                   // The texture name
        
    static Texture2D* CreateColorTexture(std::string texName, glm::u8vec4 texColor);    // Creates a 1x1 texture with the given color.
    static std::unique_ptr<Texture2D> &GetDefaultTexture();                             // Gets the a default texture white 1x1 texture.
};