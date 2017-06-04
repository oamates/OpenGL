#version 430 core

layout (location = 0, r32f) uniform image3D density_texture;
layout (local_size_x = 4, local_size_y = 4, local_size_z = 4) in;

uniform mat4 camera_matrix;

//==============================================================================================================================================================
// Main density function
//==============================================================================================================================================================
vec3 tri(vec3 x)
{
    vec3 q = abs(fract(x) - vec3(0.5));
    return clamp(q, 0.05, 0.45);
}

float sdf(vec3 p)
{
    vec3 pp = 16.0 * p;
    vec3 op = tri(1.1 * pp + tri(1.1 * pp.zxy));
    vec3 q = pp + (op - vec3(0.25)) * 0.3;
    q = cos(0.444 * q + sin(1.112 * pp.zxy));
    return length(q) - 1.05;
}

//==============================================================================================================================================================
// Variables describing view and projection matrices
//==============================================================================================================================================================
const float znear = 1.0f;
const float zfar = 7.0f;
const float tan_fov = 0.5773502691896257645f;								// tan of pi/6
const float aspect = 0.5625f;								                // aspect of 1920 x 1080 resolution
const float width  = znear * tan_fov;										// width of the near plane of the view frustrum 
const float height = width * aspect;                                 		// height of the near plane of the view frustrum

const vec3 frustrum_size = 2.0f * vec3(width, height, znear - zfar);
const vec3 bottom_left = vec3(-width, -height, -znear); 

//==============================================================================================================================================================
// Shader invocation entry point
//==============================================================================================================================================================
void main() 
{

	ivec3 cell_count = ivec3(gl_WorkGroupSize * gl_NumWorkGroups) - ivec3(1, 1, 1);
	 vec3 cell_size = frustrum_size / vec3(cell_count);
	ivec3 index = ivec3(gl_GlobalInvocationID);
	 vec3 vertex_cs = bottom_left + vec3(index) * cell_size;
	 vertex_cs.x = vertex_cs.x * (-vertex_cs.z) / znear;
	 vertex_cs.y = vertex_cs.y * (-vertex_cs.z) / znear;

     vec3 vertex_ws = vec3(camera_matrix * vec4(vertex_cs, 1.0f));

	imageStore(density_texture, index, vec4(sdf(vertex_ws), 0.0f, 0.0f, 0.0f));
}
