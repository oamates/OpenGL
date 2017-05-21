
#include "log.hpp"
#include "fbo2d.hpp"

#define MAX_AUX_BUFFERS 4

#define clamp(x, min, max) (x < min) ? min : ((x > max) ? max : x)

Framebuffer2D::Framebuffer2D(int width, int height)
    : width(width), height(height)
{
    numAuxBuffers = 0;
    auxBuffers = 0;

    glGenRenderbuffers(1, &renderBufferDepthHandle);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBufferDepthHandle);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    
    glGenFramebuffers(1, &fboHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBufferDepthHandle);
    glDrawBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Framebuffer2D::~Framebuffer2D()
{
    glDeleteFramebuffers(1, &fboHandle);
    destroyBuffers(FBO_AUX0_BIT | FBO_AUX1_BIT | FBO_AUX2_BIT | FBO_AUX3_BIT | FBO_DEPTH_BIT);
}

bool Framebuffer2D::attachBuffer(unsigned char buffer, GLint internalFormat, GLint format, GLint type, GLint textureMinFilter, GLint textureMagFilter, GLint textureWrapS, GLint textureWrapT, GLboolean mipMap)
{
    if(bufferIsValid(buffer))
    {
        unsigned int *textureHandle = getTextureHandle(buffer);
        GLenum attachment = getGLAttachment(buffer);

        if(buffer == FBO_DEPTH)
            destroyBuffers(RBO_DEPTH_BIT);

        if(glIsTexture(*textureHandle))
            glDeleteTextures(1, textureHandle);
        
        glGenTextures(1, textureHandle);

        glBindTexture(GL_TEXTURE_2D, *textureHandle);

        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureMinFilter );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureMagFilter );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrapS );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrapT );
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL);

        if(mipMap)
            glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);

        if(bufferIsAux(buffer))
             updateAuxBuffers();

        glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);

        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, *textureHandle, 0);
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return true;
    }

    debug_msg("passed buffer was not valid");
    return false;
}

void Framebuffer2D::destroyBuffers(unsigned char bufferBit)
{
    if( bufferBit & FBO_AUX0_BIT &&
        glIsTexture(bufferHandle[FBO_AUX0]))
            glDeleteTextures(1, &bufferHandle[FBO_AUX0]);

    if( bufferBit & FBO_AUX1_BIT &&
        glIsTexture(bufferHandle[FBO_AUX1]))
            glDeleteTextures(1, &bufferHandle[FBO_AUX1]);

    if( bufferBit & FBO_AUX2_BIT &&
        glIsTexture(bufferHandle[FBO_AUX2]))
            glDeleteTextures(1, &bufferHandle[FBO_AUX2]);

    if( bufferBit & FBO_AUX3_BIT &&
        glIsTexture(bufferHandle[FBO_AUX3]))
            glDeleteTextures(1, &bufferHandle[FBO_AUX3]);

    if(bufferBit & FBO_DEPTH_BIT &&
        glIsTexture(bufferHandle[FBO_DEPTH]))
            glDeleteTextures(1, &bufferHandle[FBO_DEPTH]);

    if(bufferBit & RBO_DEPTH_BIT && glIsRenderbuffer(renderBufferDepthHandle))
    {
        glDeleteRenderbuffers(1, &renderBufferDepthHandle);
    }
}

void Framebuffer2D::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);
    glViewport(0, 0, width, height);

    if(numAuxBuffers > 0)
        glDrawBuffers(numAuxBuffers, auxBuffers);
}

void Framebuffer2D::updateAuxBuffers()
{
    numAuxBuffers = 0;
    for(unsigned int i = 0; i < 4; i++)
    {
        if(glIsTexture(bufferHandle[i]))
            numAuxBuffers++;
    }

    if(auxBuffers)
        delete[] auxBuffers;

    auxBuffers = new GLenum[numAuxBuffers];

    for(unsigned int i = 0; i < numAuxBuffers; i++)
    {
        auxBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
    }
}

bool Framebuffer2D::bufferIsAux(unsigned char buffer)
    { return buffer == FBO_AUX0 || buffer == FBO_AUX1 || buffer == FBO_AUX2 || buffer == FBO_AUX3; }

bool Framebuffer2D::bufferIsDepth(unsigned char buffer)
    { return buffer == FBO_DEPTH; }

bool Framebuffer2D::bufferIsValid(unsigned char buffer)
    { return buffer == FBO_AUX0 || buffer == FBO_AUX1 || buffer == FBO_AUX2 || buffer == FBO_AUX3 || buffer == FBO_DEPTH; }

unsigned int *Framebuffer2D::getTextureHandle(unsigned char buffer)
{
    buffer = clamp(buffer,0,FBO_COUNT-1);
    return &bufferHandle[buffer];
}

GLenum Framebuffer2D::getGLAttachment(unsigned char buffer)
{
    static unsigned short attachmentMap[FBO_COUNT] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3,
        GL_DEPTH_ATTACHMENT
    };

    // ASSERT THIS!
    buffer = clamp(buffer, 0, FBO_COUNT - 1);

    return attachmentMap[buffer];
}

#undef clamp