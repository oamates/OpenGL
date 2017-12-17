#pragma once

#include <set>
#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/detail/type_vec1.hpp>
#include <glm/detail/type_vec3.hpp>
#include <glm/detail/type_vec4.hpp>

#include <oglplus/texture.hpp>
#include <oglplus/context.hpp>
#include <oglplus/bound/texture.hpp>

#include "../types/base_object.hpp"
#include "../make_unique.hpp"

struct RawTexture : public BaseObject
{
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

    std::string filepath;
    unsigned int height;
    unsigned int width;
    unsigned int depth;
    unsigned int lineWidth;
    unsigned int bitsPerPixel;
    std::unique_ptr<unsigned char> rawData;
    std::set<TextureType> textureTypes;

    RawTexture() : height(1), width(1), depth(1), lineWidth(1), bitsPerPixel(1), rawData(nullptr) { }

    ~RawTexture()
    {
        if (this->rawData)
            free(this->rawData.release());
    }
        
    bool IsType(TextureType type)                                   // Determines the texture is of the given type.
        { return textureTypes.find(type) != textureTypes.end(); }

    std::string GetFilepath() const                                 // Gets the texture filepath.
        { return filepath; }

    RawTexture(RawTexture const &) = delete;
    RawTexture& operator = (RawTexture const &) = delete;
};

struct Texture2D : public RawTexture
{
    std::unique_ptr<oglplus::Texture> oglTexture;

    Texture2D() {}
    ~Texture2D() {}

    Texture2D(Texture2D const &) = delete;
    Texture2D &operator = (Texture2D const &) = delete;

    // Loads the associated texture file with the given parameters.
    void Load(oglplus::TextureMinFilter minFilter, oglplus::TextureMagFilter magFilter,
              oglplus::TextureWrap wrapS, oglplus::TextureWrap wrapT,
              bool generateMipmaps = true, glm::vec4 borderColor = glm::vec4(0.0f))
    {
        static oglplus::Context gl;
        if (this->oglTexture || !this->rawData)                                         // already loaded a texture or no texture data to create
            return;
    
        using namespace oglplus;
        auto pdf = PixelDataFormat::BGRA;
        auto pdif = PixelDataInternalFormat::RGBA8;
        this->oglTexture = std::make_unique<Texture>();
        unsigned int bytesPerPixel = this->bitsPerPixel / 8;

        pdf  = bytesPerPixel == 3 ? PixelDataFormat::BGR : pdf;
        pdif = bytesPerPixel == 3 ? PixelDataInternalFormat::RGB8 : pdif;
        pdf  = bytesPerPixel == 2 ? PixelDataFormat::RG : pdf;
        pdif = bytesPerPixel == 2 ? PixelDataInternalFormat::RG8 : pdif;
        pdf  = bytesPerPixel == 1 ? PixelDataFormat::Red : pdf;
        pdif = bytesPerPixel == 1 ? PixelDataInternalFormat::R8 : pdif;

        gl.Bound(TextureTarget::_2D, *this->oglTexture).Image2D(0, pdif, this->width, this->height, 0, pdf, PixelDataType::UnsignedByte, this->rawData.get());
        if (bytesPerPixel == 1)
            Texture::SwizzleRGBA(TextureTarget::_2D, TextureSwizzle::Red, TextureSwizzle::Red, TextureSwizzle::Red, TextureSwizzle::Red);
        gl.Bound(TextureTarget::_2D, *this->oglTexture).MinFilter(minFilter).MagFilter(magFilter).WrapS(wrapS).WrapT(wrapT);
    
        if (wrapS == TextureWrap::ClampToBorder || wrapT == TextureWrap::ClampToBorder)
        {
            Vector<float, 4> color(borderColor.x, borderColor.y, borderColor.z, borderColor.w);
            gl.Bound(TextureTarget::_2D, *this->oglTexture).BorderColor(color);
        }
    
        
        if (generateMipmaps)                                                            // generate mipmaps
            gl.Bound(TextureTarget::_2D, *this->oglTexture).GenerateMipmap();
    
        free(this->rawData.release());
    }
        
    void Bind() const                                                                   // Binds the texture.
        { this->oglTexture->Bind(oglplus::Texture::Target::_2D); }

    int Name() const                                                                    // The texture name
        { return oglplus::GetName(*oglTexture); }
        
    static Texture2D* CreateColorTexture(std::string texName, glm::u8vec4 texColor)     // Creates a 1x1 texture with the given color.
    {
        auto defaultTexture = new Texture2D();
        defaultTexture->filepath = texName;
        defaultTexture->width = 1;
        defaultTexture->height = 1;
        defaultTexture->lineWidth = 1;
        defaultTexture->depth = 1;
        defaultTexture->bitsPerPixel = 32;
        defaultTexture->rawData.reset(new unsigned char[4] { texColor.b, texColor.g, texColor.r, texColor.a });
        for (unsigned int i = 0; i < TYPE_MAX; ++i)                                     // texture types conveyed by default
            defaultTexture->textureTypes.insert(TextureType(None + i));
        return defaultTexture;
    }

    static std::unique_ptr<Texture2D> &GetDefaultTexture()                              // Gets the a default texture white 1x1 texture.
    {
        static std::unique_ptr<Texture2D> instance = 0;
        if (!instance)
        {
            std::string texName = "!defaultTexture";                                    // default texture is white
            glm::u8vec4 texColor = glm::u8vec4(255, 255, 255, 255);
            instance.reset(CreateColorTexture(texName, texColor));                      // save to instance
            instance->Load(oglplus::TextureMinFilter::LinearMipmapLinear, oglplus::TextureMagFilter::Linear, oglplus::TextureWrap::Repeat, oglplus::TextureWrap::Repeat, false);
        }
        return instance;
    }
};