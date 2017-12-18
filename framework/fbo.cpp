//=======================================================================================================================================================================================================================
// Framebuffer object methods implementation
//=======================================================================================================================================================================================================================

#include <GL/glew.h>
#include "fbo.hpp"
#include "log.hpp"

//=======================================================================================================================================================================================================================
// fbo_color : FBO with just color attachments of type [target]
// [target] should be one of the :: GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_1D_ARRAY, GL_TEXTURE_2D_ARRAY,
//                                  GL_TEXTURE_RECTANGLE, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_BUFFER,
//                                  GL_TEXTURE_2D_MULTISAMPLE or GL_TEXTURE_2D_MULTISAMPLE_ARRAY.
// [color_attachments] is the number of the color attachments.
//=======================================================================================================================================================================================================================

template<GLenum target, unsigned int color_attachments>
fbo_color_t<target, color_attachments>::fbo_color_t() : id(0)
{
    for(unsigned int attachment = 0; attachment < color_attachments; ++attachment)
        texture_id[attachment] = 0;
}

template<GLenum target, unsigned int color_attachments>
fbo_color_t<target, color_attachments>::fbo_color_t(GLsizei res_x, GLsizei res_y, GLenum internal_format, GLint wrap_mode)
{
	debug_msg("Creating color FBO with (%d x %d) attachment. Target :: %u, internal format :: %u", res_x, res_y, target, internal_format);

	fbo_color_t::res_x = res_x;
	fbo_color_t::res_y = res_y;
	glGenFramebuffers(1, &id);
	glBindFramebuffer(GL_FRAMEBUFFER, id);


	glGenTextures(color_attachments, texture_id);

    GLenum buffers[color_attachments];
    for(unsigned int attachment = 0; attachment < color_attachments; ++attachment)
    {
    	glBindTexture(target, texture_id[attachment]);
	    glTexStorage2D(target, 1, internal_format, res_x, res_y);

    	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    	glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap_mode);
    	glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap_mode);
	    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment, texture_id[attachment], 0);
        buffers[attachment] = GL_COLOR_ATTACHMENT0 + attachment;
    }

    glDrawBuffers(color_attachments, buffers);
    check_status();

	glViewport(0, 0, res_x, res_y);
	debug_msg("Color FBO created. id = %d.", id);
}

template<GLenum target, unsigned int color_attachments>
void fbo_color_t<target, color_attachments>::reset_textures(GLsizei res_x, GLsizei res_y, GLenum internal_format, GLint wrap_mode)
{
	debug_msg("Resetting textures for color FBO %d with (%d x %d) attachment. Target :: %u, internal format :: %u", id, res_x, res_y, target, internal_format);

	fbo_color_t::res_x = res_x;
	fbo_color_t::res_y = res_y;

	glGenTextures(color_attachments, texture_id);

    for(unsigned int attachment = 0; attachment < color_attachments; ++attachment)
    {
    	glBindTexture(target, texture_id[attachment]);
	    glTexStorage2D(target, 1, internal_format, res_x, res_y);

    	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    	glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap_mode);
    	glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap_mode);
	    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment, texture_id[attachment], 0);
    }
    check_status();

	glViewport(0, 0, res_x, res_y);
	debug_msg("Color FBO reset successful. id = %d.", id);
}

template<GLenum target, unsigned int color_attachments>
void fbo_color_t<target, color_attachments>::bind()
    { glBindFramebuffer(GL_FRAMEBUFFER, id); }

template<GLenum target, unsigned int color_attachments>
void fbo_color_t<target, color_attachments>::bind_texture(GLenum texture_unit, GLenum attachment)
{
	glActiveTexture(texture_unit);
	glBindTexture(GL_TEXTURE_2D, texture_id[attachment]);
}

template<GLenum target, unsigned int color_attachments>
fbo_color_t<target, color_attachments>::~fbo_color_t()
{
	glDeleteTextures(color_attachments, texture_id);
	glDeleteFramebuffers(1, &id);
}

template struct fbo_color_t<GL_TEXTURE_2D, 1>;
template struct fbo_color_t<GL_TEXTURE_2D, 2>;

//=======================================================================================================================================================================================================================
// color + renderbuffer structure
// ======================================================================================================================================================================================================================

color_rbo_t::color_rbo_t(GLuint width, GLuint height)
{
    glGenFramebuffers(1, &fbo_id);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

    glGenTextures(1, &color_texture_id);
    glBindTexture(GL_TEXTURE_2D, color_texture_id);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
 	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color_texture_id, 0);

    glGenRenderbuffers(1, &rbo_id);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_id);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_id);

	glViewport(0, 0, width, height); 																					// Render on the whole framebuffer, complete from the lower left corner to the upper right
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    check_status();
}

color_rbo_t::~color_rbo_t()
{
	glDeleteTextures(1, &color_texture_id);
    glDeleteRenderbuffers(1, &rbo_id);
    glDeleteFramebuffers(1, &fbo_id);
}


void color_rbo_t::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
}

void color_rbo_t::bind_texture(GLenum texture_unit)
{
	glActiveTexture (texture_unit);
	glBindTexture(GL_TEXTURE_2D, color_texture_id);
}

//========================================================================================================================================================================================================================
// static function to check the framebuffer status
//========================================================================================================================================================================================================================
void check_status()
{
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (GL_FRAMEBUFFER_COMPLETE == status)
    {
        debug_msg("GL_FRAMEBUFFER is COMPLETE.");
        return;
    }

	const char * msg;
	switch (status)
	{
		case GL_FRAMEBUFFER_UNDEFINED:                     msg = "GL_FRAMEBUFFER_UNDEFINED."; break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:         msg = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT."; break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: msg = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT."; break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:        msg = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER."; break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:        msg = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER."; break;
		case GL_FRAMEBUFFER_UNSUPPORTED:                   msg = "GL_FRAMEBUFFER_UNSUPPORTED."; break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:        msg = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE."; break;
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:      msg = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS."; break;
	  default:
		msg = "Unknown Framebuffer error.";
	}

	exit_msg("FBO incomplete : %s", msg);
}