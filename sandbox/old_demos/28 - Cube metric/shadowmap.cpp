//=======================================================================================================================================================================================================================
//  Cube-mapped Z and distance texture render structure methods implementation
//=======================================================================================================================================================================================================================

#include <GL/glew.h> 														                                                // OpenGL extensions
#include "shadowmap.hpp"
#include "log.hpp"

shadow_cubemap::shadow_cubemap() : texture_size(2048)
{
    glGenFramebuffers(1, &fbo_id);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

    // Create the depth buffer
    glGenRenderbuffers(1, &depth_buffer_id);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer_id);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, texture_size, texture_size);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer_id);

    glGenTextures(1, &shadow_texture_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, shadow_texture_id);
	glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_R32F, texture_size, texture_size);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
 	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, shadow_texture_id, 0);

	glViewport(0, 0, texture_size, texture_size); 																					// Render on the whole framebuffer, complete from the lower left corner to the upper right

	glDrawBuffer(GL_COLOR_ATTACHMENT0);				
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (GL_FRAMEBUFFER_COMPLETE == status) 
	{
		debug_msg("GL_FRAMEBUFFER %u object succesfully initialized.", fbo_id);
		return;
	};

	debug_msg("Could not initialize GL_FRAMEBUFFER object. Error status : 0x%x", status);
};

shadow_cubemap::~shadow_cubemap()
{
	glDeleteTextures(1, &shadow_texture_id);
	glDeleteRenderbuffers(1, &depth_buffer_id);
    glDeleteFramebuffers(1, &fbo_id);
};

void shadow_cubemap::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
	float INFINITY[] = {50000.0, 50000.0, 50000.0, 1.0};
	glClear(GL_DEPTH_BUFFER_BIT);
	glClearBufferfv(GL_COLOR, 0, INFINITY);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
};

void shadow_cubemap::bind_texture(GLenum texture_unit)
{
	glActiveTexture (texture_unit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, shadow_texture_id);
};
