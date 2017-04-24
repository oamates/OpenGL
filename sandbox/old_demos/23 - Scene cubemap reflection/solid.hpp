struct solid
{
    // mesh_size is the amount of triangles
    unsigned int mesh_size;
    GLuint vao_id;
    GLuint vbo_id, nbo_id, tbo_id;

    GLuint texture_id, normal_texture_id;    

    solid(const glm::vec3* positions, const glm::vec3* normals, const glm::vec2* texture_coords, unsigned int mesh_size) : mesh_size(mesh_size)
    {
        glGenVertexArrays(1, &vao_id);
        glBindVertexArray(vao_id);
        glEnableVertexAttribArray(0);
        glGenBuffers(1, &vbo_id);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
        glBufferData(GL_ARRAY_BUFFER, mesh_size * sizeof(glm::vec3), positions, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glEnableVertexAttribArray(1);
    	glGenBuffers(1, &nbo_id);
    	glBindBuffer(GL_ARRAY_BUFFER, nbo_id);
    	glBufferData(GL_ARRAY_BUFFER, mesh_size * sizeof(glm::vec3), normals, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glEnableVertexAttribArray(2);
    	glGenBuffers(1, &tbo_id);
    	glBindBuffer(GL_ARRAY_BUFFER, tbo_id);
    	glBufferData(GL_ARRAY_BUFFER, mesh_size * sizeof(glm::vec2), texture_coords, GL_STATIC_DRAW);
    	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

    };

    void textures(GLuint tex_id, GLuint normal_tex_id) 
    {
        texture_id = tex_id;
        normal_texture_id = normal_tex_id;
    };

    void bind()
    {
        glActiveTexture(GL_TEXTURE0);																										// bind our texture in texture unit 0
		glBindTexture(GL_TEXTURE_2D, texture_id);
        glActiveTexture(GL_TEXTURE1);																										// bind our texture in texture unit 0
		glBindTexture(GL_TEXTURE_2D, normal_texture_id);
        glBindVertexArray(vao_id);
    };

    ~solid()
    {
    	glDeleteBuffers(1, &vbo_id);																					// cleanup vertex buffers and shader
    	glDeleteBuffers(1, &nbo_id);
    	glDeleteBuffers(1, &tbo_id);
    	glDeleteVertexArrays(1, &vao_id);
    };
};
