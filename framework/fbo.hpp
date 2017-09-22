#ifndef _fbo_included_360378610346107567840634150345634056734781563452012923402
#define _fbo_included_360378610346107567840634150345634056734781563452012923402

//========================================================================================================================================================================================================================
// Framebuffer object structures with different configurations :
//
//  * fbo_depth               : FBO with a single texture of GL_DEPTH_ATTACHMENT format attachment
//  * fbo_color               : FBO with a single GL_COLOR_ATTACHMENT0 of type GL_TEXTURE_2D
//  * msfbo_color             : multisampled FBO with a single GL_COLOR_ATTACHMENT0 of type GL_TEXTURE_2D
//  * fbo_cubemap             : FBO with a single GL_COLOR_ATTACHMENT0 of type GL_TEXTURE_CUBEMAP
//  * msfbo_cubemap           : FBO with a single GL_COLOR_ATTACHMENT0 of type GL_TEXTURE_CUBEMAP
//  * fbo_depth_cubemap       : FBO with a single GL_DEPTH_ATTACHMENT of type GL_TEXTURE_CUBEMAP
//  * fbo_depth_color         : FBO with both GL_DEPTH_ATTACHMENT and GL_COLOR_ATTACHMENT0 of type GL_TEXTURE_2D, extra GL_COLOR_ATTACHMENTi can be added
//  * fbo_depth_color_cubemap : FBO with both GL_DEPTH_ATTACHMENT and GL_COLOR_ATTACHMENT0 of type GL_TEXTURE_CUBEMAP, extra GL_COLOR_ATTACHMENTi can be added
//========================================================================================================================================================================================================================

#include <GL/glew.h> 														                                                // OpenGL extensions
#include "log.hpp"

void check_status();

//=======================================================================================================================================================================================================================
// fbo_depth : FBO with a single depth texture attached
//=======================================================================================================================================================================================================================
//=======================================================================================================================================================================================================================
// Setup 5 :: framebuffer with a single depth
//=======================================================================================================================================================================================================================
struct fbo_depth_t
{
    GLsizei res_x, res_y;

    GLuint id;
    GLuint texture_id;
    
    fbo_depth_t(GLsizei res_x, GLsizei res_y, GLenum texture_unit, GLenum internal_format, GLint filtering_mode, GLint wrap_mode)
        : res_x(res_x), res_y(res_y)
    {
        debug_msg("Creating FBO with one %dx%d depth attachment. Internal format :: %u", res_x, res_y, internal_format);

        glGenFramebuffers(1, &id);
        glBindFramebuffer(GL_FRAMEBUFFER, id);

        glActiveTexture(texture_unit);
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
    
        glTexStorage2D(GL_TEXTURE_2D, 1, internal_format, res_x, res_y);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering_mode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering_mode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_mode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_mode);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture_id, 0);

        check_status();
    }
    
    //===================================================================================================================================================================================================================
    // target should be one of GL_FRAMEBUFFER, GL_DRAW_FRAMEBUFFER, GL_READ_FRAMEBUFFER
    //===================================================================================================================================================================================================================
    void bind(GLenum target)
        { glBindFramebuffer(target, id); }
    
    ~fbo_depth_t() 
    {
        glDeleteTextures(1, &texture_id);
        glDeleteFramebuffers(1, &id);
    }
};

//=======================================================================================================================================================================================================================
// fbo_color : FBO with just color attachments of type [target]
// [target] should be one of the :: GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_1D_ARRAY, GL_TEXTURE_2D_ARRAY, 
//                                  GL_TEXTURE_RECTANGLE, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_BUFFER, 
//                                  GL_TEXTURE_2D_MULTISAMPLE or GL_TEXTURE_2D_MULTISAMPLE_ARRAY.
// [color_attachments] is the number of the color attachments.
//=======================================================================================================================================================================================================================
template<GLenum target, unsigned int color_attachments> struct fbo_color_t
{
	GLuint id;
	GLuint texture_id[color_attachments];
	GLsizei res_x, res_y;

	fbo_color_t();
	fbo_color_t(GLsizei width, GLsizei height, GLenum internal_format = GL_RGBA32F, GLint wrap_mode = GL_REPEAT);
    void reset_textures(GLsizei res_x, GLsizei res_y, GLenum internal_format = GL_RGBA32F, GLint wrap_mode = GL_REPEAT);


	void bind();
	void bind_texture(GLenum texture_unit, GLuint attachment = 0);
	~fbo_color_t();
};

//=======================================================================================================================================================================================================================
// color + renderbuffer structure
//=======================================================================================================================================================================================================================
struct color_rbo_t
{
	color_rbo_t(GLuint width, GLuint height);
    ~color_rbo_t();

    void bind();
    void bind_texture(GLenum texture_unit);

    GLuint fbo_id;
    GLuint rbo_id;
	GLuint color_texture_id;
};

struct fbo_color_cubemap
{
	GLuint id;
	GLuint texture_id;

	fbo_color_cubemap() : id(0), texture_id(0) {};

	fbo_color_cubemap(GLsizei texture_size, GLenum internal_format = GL_RGB)
	{
		//================================================================================================================================================================================================================
		// create framebuffer with one GL_TEXTURE_CUBE_MAP attached to GL_COLOR_ATTACHMENT0 point
		//================================================================================================================================================================================================================
		glGenFramebuffers(1, &id);
		glBindFramebuffer(GL_FRAMEBUFFER, id);
		glGenTextures(1, &texture_id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
		glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, internal_format, texture_size, texture_size);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	 	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture_id, 0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);	
		viewport(0, 0, texture_size, texture_size);

		//================================================================================================================================================================================================================
		// check that the created framebuffer object is ok
		//================================================================================================================================================================================================================
	    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	    	exit_msg("Could not initialize GL_FRAMEBUFFER object.");
	}

	static void viewport(GLint x, GLint y, GLsizei width, GLsizei height)
		{ glViewport(x, y, width, height); }

	void bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, id);
		glClear(GL_COLOR_BUFFER_BIT);		
	}

	void bind_texture(GLenum texture_unit)
	{
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
	}

	~fbo_color_cubemap() 
	{
		glDeleteTextures(1, &texture_id);
		glDeleteFramebuffers(1, &id);
	}	
};

struct fbo_depth_cubemap
{
    GLuint id;
    GLuint texture_id;

    fbo_depth_cubemap() : id(0), texture_id(0) {};

    fbo_depth_cubemap(GLsizei texture_size, GLenum internal_format = GL_DEPTH_COMPONENT32)
    {
    	debug_msg("Creating depth only FBO with GL_TEXTURE_CUBE_MAP attachment.");
	    glGenFramebuffers(1, &id);
    	glBindFramebuffer(GL_FRAMEBUFFER, id);

	    glGenTextures(1, &texture_id);
    	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
		glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, internal_format, texture_size, texture_size);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

//    	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//    	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
 		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture_id, 0);
		viewport(0, 0, texture_size, texture_size);

		//================================================================================================================================================================================================================
		// check that the created framebuffer object is ok
		//================================================================================================================================================================================================================
	    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	    	exit_msg("Could not initialize GL_FRAMEBUFFER object.");

    	debug_msg("Done. FBO id = %d. texture_id = %d", id, texture_id);
    }

	static void viewport(GLint x, GLint y, GLsizei width, GLsizei height)
		{ glViewport(x, y, width, height); }    

    void bind()
    {
	    glBindFramebuffer(GL_FRAMEBUFFER, id);
		glClear(GL_DEPTH_BUFFER_BIT);
    }

    void bind_texture(GLenum texture_unit)
    {
		glActiveTexture (texture_unit);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
    }

    ~fbo_depth_cubemap()
    {
		glDeleteTextures(1, &texture_id);
	    glDeleteFramebuffers(1, &id);    	
    }
};

struct fbo_depth_color
{

};

#endif // _fbo_included_360378610346107567840634150345634056734781563452012923402


