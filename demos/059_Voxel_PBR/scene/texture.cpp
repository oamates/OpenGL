#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <oglplus/context.hpp>
#include <oglplus/bound/texture.hpp>
#include "texture.hpp"

#include <glm/detail/type_vec3.hpp>

std::string RawTexture::GetFilepath() const
    { return filepath; }

bool RawTexture::IsType(TextureType type)
    { return textureTypes.find(type) != textureTypes.end(); }

RawTexture::RawTexture() :
    height(1), width(1), depth(1), lineWidth(1),
    bitsPerPixel(1), rawData(nullptr)
    { }


RawTexture::~RawTexture()
{
    if (!this->rawData)
        return;
    free(this->rawData.release());
}

void Texture2D::Load(oglplus::TextureMinFilter minFilter,
                     oglplus::TextureMagFilter magFilter,
                     oglplus::TextureWrap wrapS,
                     oglplus::TextureWrap wrapT,
                     bool generateMipmaps,
                     glm::vec4 borderColor)
{
    static oglplus::Context gl;

    if (this->oglTexture || !this->rawData)                             // already loaded a texture or no texture data to create
        return;

    using namespace oglplus;
    auto pdf = PixelDataFormat::BGRA;
    auto pdif = PixelDataInternalFormat::RGBA8;
    this->oglTexture = std::make_unique<Texture>();
    unsigned int bytesPerPixel = this->bitsPerPixel / 8;
    // proper data format
    pdf = bytesPerPixel == 3 ? PixelDataFormat::BGR : pdf;
    pdif = bytesPerPixel == 3 ? PixelDataInternalFormat::RGB8 : pdif;
    pdf = bytesPerPixel == 2 ? PixelDataFormat::RG : pdf;
    pdif = bytesPerPixel == 2 ? PixelDataInternalFormat::RG8 : pdif;
    pdf = bytesPerPixel == 1 ? PixelDataFormat::Red : pdf;
    pdif = bytesPerPixel == 1 ? PixelDataInternalFormat::R8 : pdif;
    // create texture with raw data (upload to gpu)
    gl.Bound(TextureTarget::_2D, *this->oglTexture).Image2D(0, pdif, this->width, this->height, 0, pdf, PixelDataType::UnsignedByte, this->rawData.get());

    if (bytesPerPixel == 1)
    {
        Texture::SwizzleRGBA(TextureTarget::_2D, TextureSwizzle::Red,
                             TextureSwizzle::Red, TextureSwizzle::Red,
                             TextureSwizzle::Red);
    }

    // opengl texture parameters
    gl.Bound(TextureTarget::_2D, *this->oglTexture).MinFilter(minFilter).MagFilter(magFilter).WrapS(wrapS).WrapT(wrapT);

    if (wrapS == TextureWrap::ClampToBorder || wrapT == TextureWrap::ClampToBorder)
    {
        Vector<float, 4> color(borderColor.x, borderColor.y, borderColor.z,
                               borderColor.w);
        gl.Bound(TextureTarget::_2D, *this->oglTexture).BorderColor(color);
    }

    // gen mipmaps
    if (generateMipmaps)
    {
        gl.Bound(TextureTarget::_2D, *this->oglTexture).GenerateMipmap();
    }

    free(this->rawData.release());
}

void Texture2D::Bind() const
{
    this->oglTexture->Bind(oglplus::Texture::Target::_2D);
}

int Texture2D::Name() const
    { return oglplus::GetName(*oglTexture); }

Texture2D * Texture2D::CreateColorTexture(std::string texName,
        glm::u8vec4 texColor)
{
    auto defaultTexture = new Texture2D();
    defaultTexture->filepath = texName;
    defaultTexture->width = 1;
    defaultTexture->height = 1;
    defaultTexture->lineWidth = 1;
    defaultTexture->depth = 1;
    defaultTexture->bitsPerPixel = 32;
    defaultTexture->rawData.reset(new unsigned char[4]
    {
        texColor.b, texColor.g, texColor.r, texColor.a
    });

    // texture types conveyed by default
    for (unsigned int i = 0; i < TYPE_MAX; ++i)
        defaultTexture->textureTypes.insert(TextureType(None + i));

    return defaultTexture;
}

std::unique_ptr<Texture2D> &Texture2D::GetDefaultTexture()
{
    static std::unique_ptr<Texture2D> instance = 0;

    if (!instance)
    {
        // default texture is white
        std::string texName = "!defaultTexture";
        glm::u8vec4 texColor = glm::u8vec4(255, 255, 255, 255);
        // save to instance
        instance.reset(CreateColorTexture(texName, texColor));
        instance->Load
        (
            oglplus::TextureMinFilter::LinearMipmapLinear,
            oglplus::TextureMagFilter::Linear,
            oglplus::TextureWrap::Repeat,
            oglplus::TextureWrap::Repeat, false
        );
    }

    return instance;
}