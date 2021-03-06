#ifndef _shadowmap_included_360378610346107567840634150345634056734781563452012   
#define _shadowmap_included_360378610346107567840634150345634056734781563452012

//=======================================================================================================================================================================================================================
// shadow + depth buffer structure
// ======================================================================================================================================================================================================================
struct shadow_cubemap
{
	shadow_cubemap(GLuint light_sources, GLuint texture_size = 2048);
    ~shadow_cubemap();

    void bind();
    void bind_texture(GLenum texture_unit);

    GLuint fbo_id;
    GLuint shadow_texture_id;
	GLuint depth_buffer_id;
    unsigned int texture_size;
};

//=======================================================================================================================================================================================================================
// depth buffer structure, no color attachment
// ======================================================================================================================================================================================================================
struct depth_map
{
	depth_map(GLuint width, GLuint height);
    ~depth_map();

    void bind();
    void bind_texture(GLenum texture_unit);

    GLuint fbo_id;
	GLuint depth_texture_id;
};

//=======================================================================================================================================================================================================================
// color buffer structure, no depth attachment
// ======================================================================================================================================================================================================================
struct color_map
{
	color_map(GLuint width, GLuint height);
    ~color_map();

    void bind();
    void bind_texture(GLenum texture_unit);

    GLuint fbo_id;
	GLuint color_texture_id;
};

#endif // _shadowmap_included_360378610346107567840634150345634056734781563452012


