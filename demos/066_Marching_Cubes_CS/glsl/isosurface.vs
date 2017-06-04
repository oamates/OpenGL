#version 430 core

uniform mat4 camera_matrix;

const float znear = 1.0f;
const float zfar = 7.0f;
const float tan_fov = 0.5773502691896257645f;								// tan of pi/6
const float aspect = 0.5625f;								                // aspect of 1920 x 1080 resolution
const float width  = znear * tan_fov;										// width of the near plane of the view frustrum 
const float height = width * aspect;                                 		// height of the near plane of the view frustrum

const vec3 frustrum_size = 2.0f * vec3(width, height, znear - zfar);
const vec3 bottom_left = vec3(-width, -height, -znear); 


const mat4 projection_matrix = mat4(vec4(znear / width,           0.0f,                                  0.0f,  0.0f), 
                                    vec4(         0.0f, znear / height,                                  0.0f,  0.0f),
                                    vec4(         0.0f,           0.0f,      -(znear + zfar) / (zfar - znear), -1.0f),
                                    vec4(         0.0f,           0.0f, - 2.0f * zfar * znear/ (zfar - znear),  0.0f));

layout (std430, binding = 0) buffer shader_data
{
	vec3 position[];
};

out vec3 position_ws;
out vec3 position_cs;

void main()
{
	position_cs = position[gl_VertexID];
	position_ws = vec3(camera_matrix * vec4(position_cs, 1.0f)); 
	gl_Position = projection_matrix * vec4(position_cs, 1.0f);
}
