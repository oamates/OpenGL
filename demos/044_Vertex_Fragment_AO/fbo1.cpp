//=======================================================================================================================================================================================================================
//  Cube-mapped Z and distance texture render structure methods implementation
//=======================================================================================================================================================================================================================

#include <GL/glew.h> 														                                                // OpenGL extensions
#include "fbo1.hpp"
#include "log.hpp"

//=======================================================================================================================================================================================================================
// depth buffer structure, no color attachment
//=======================================================================================================================================================================================================================

depth_map_t::depth_map_t(GLuint width, GLuint height, GLenum texture_unit, GLenum internal_format)
{
    glGenFramebuffers(1, &fbo_id);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

    glActiveTexture(texture_unit);
    glGenTextures(1, &depth_texture_id);
    glBindTexture(GL_TEXTURE_2D, depth_texture_id);
	glTexStorage2D(GL_TEXTURE_2D, 1, internal_format, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
 	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_texture_id, 0);

    glDrawBuffer(GL_NONE);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (GL_FRAMEBUFFER_COMPLETE == status) 
	{ 
		debug_msg("GL_FRAMEBUFFER %u object succesfully initialized.", fbo_id);
		return;
	}

	debug_msg("Could not initialize GL_FRAMEBUFFER object. Error status : 0x%x", status);
}

depth_map_t::~depth_map_t()
{
	glDeleteTextures(1, &depth_texture_id);
    glDeleteFramebuffers(1, &fbo_id);
}

void depth_map_t::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
}

void depth_map_t::bind_texture(GLenum texture_unit)
{
	glActiveTexture (texture_unit);
	glBindTexture(GL_TEXTURE_2D, depth_texture_id);
}


//=======================================================================================================================================================================================================================
// color buffer structure, no depth attachment
//=======================================================================================================================================================================================================================
color_map_t::color_map_t(GLuint width, GLuint height, GLenum texture_unit, GLenum internal_format)
{
    glGenFramebuffers(1, &fbo_id);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

    glActiveTexture(texture_unit);
    glGenTextures(1, &color_texture_id);
    glBindTexture(GL_TEXTURE_2D, color_texture_id);
	glTexStorage2D(GL_TEXTURE_2D, 1, internal_format, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
 	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color_texture_id, 0);

    glDrawBuffer(GL_COLOR_ATTACHMENT0);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (GL_FRAMEBUFFER_COMPLETE == status) 
	{ 
		debug_msg("GL_FRAMEBUFFER %u object succesfully initialized.", fbo_id);
		return;
	};

	debug_msg("Could not initialize GL_FRAMEBUFFER object. Error status : 0x%x", status);
}

color_map_t::~color_map_t()
{
	glDeleteTextures(1, &color_texture_id);
    glDeleteFramebuffers(1, &fbo_id);
}

void color_map_t::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
}

void color_map_t::bind_texture(GLenum texture_unit)
{
	glActiveTexture (texture_unit);
	glBindTexture(GL_TEXTURE_2D, color_texture_id);
}