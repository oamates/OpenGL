#version 430 core

layout (location = 0, r32f) uniform sampler3D density_texture;

uniform mat4 view_matrix;

const float znear = 1.0f;
const float tan_fov = 0.5773502691896257645f;								// tan of pi/6
const float aspect = 0.5625f;								                // aspect of 1920 x 1080 resolution
const float width  = znear * tan_fov;										// width of the near plane of the view frustrum 
const float height = width * aspect;                                 		// height of the near plane of the view frustrum

const mat4 projection_matrix = mat4(vec4(znear / width,           0.0f,           0.0f,  0.0f), 
                                    vec4(         0.0f, znear / height,           0.0f,  0.0f),
                                    vec4(         0.0f,           0.0f,          -1.0f, -1.0f),
                                    vec4(         0.0f,           0.0f, - 2.0f * znear,  0.0f));

const vec3 div_size = vec3(255.0f);



out float density;

void main()
{
	

    vec3 texture_coord = vec3(
							float(gl_VertexID & 0xFF), 
							float((gl_VertexID >> 8) & 0xFF), 
							float((gl_VertexID >> 16) & 0xFF)
						 ) / div_size;

	density = texture(density_texture, texture_coord).r;
	gl_Position = projection_matrix * view_matrix * vec4(5.0f * (texture_coord - vec3(0.5f)), 1.0f);


};
