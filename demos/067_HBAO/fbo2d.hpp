#ifndef FRAMEBUFFER2D_H
#define FRAMEBUFFER2D_H

#include "types.hpp"

struct Framebuffer2D
{
	Framebuffer2D(int width, int height);
	~Framebuffer2D();

	bool attachBuffer(unsigned char buffer,
					  GLint internalFormat = 	GL_RGBA,
					  GLint format = 			GL_RGBA,
					  GLint type = 				GL_UNSIGNED_INT,
					  GLint textureMinFilter = 	GL_NEAREST,
					  GLint textureMagFilter = 	GL_NEAREST,
					  GLint textureWrapS = 		GL_CLAMP_TO_EDGE,
					  GLint textureWrapT = 		GL_CLAMP_TO_EDGE,
					  GLboolean mipMap =		GL_FALSE);

	void destroyBuffers(unsigned char bufferBit);

	void resizeBuffers(unsigned char bufferBit, int width, int height);

	inline int getWidth() { return width; }
	inline int getHeight() { return height; }

	void bind();
	void unbind();

	inline unsigned int getBufferHandle(unsigned char buffer) { return *getTextureHandle(buffer); }

	bool bufferIsAux(unsigned char buffer);
	bool bufferIsDepth(unsigned char buffer);
	bool bufferIsValid(unsigned char buffer);
	unsigned int *getTextureHandle(unsigned char buffer);
	GLenum getGLAttachment(unsigned char buffer);
	void updateAuxBuffers();

	int width, height;

	GLenum *auxBuffers;
	unsigned char numAuxBuffers;

	unsigned int bufferHandle[5];

	unsigned int renderBufferDepthHandle;

	unsigned int fboHandle;
};

// Table 1: Buffers that can be specified in assignBuffer
#define FBO_AUX0	0x00
#define FBO_AUX1	0x01
#define FBO_AUX2	0x02
#define FBO_AUX3	0x03
#define FBO_DEPTH	0x04
#define FBO_COUNT	0x05

// Table 2: Buffer bits that can be specified in resizeBuffers
#define FBO_AUX0_BIT 	0x00
#define FBO_AUX1_BIT 	0x01
#define FBO_AUX2_BIT 	0x02
#define FBO_AUX3_BIT 	0x04
#define FBO_DEPTH_BIT	0x08
#define RBO_DEPTH_BIT	0x10


#endif