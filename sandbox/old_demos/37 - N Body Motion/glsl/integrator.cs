#version 430 core
layout(local_size_x = 256) in;

layout(location = 0) uniform float dt;

layout(std430, binding = 0) buffer pblock
{
	vec4 positions[]; 
};

layout(std430, binding = 1) buffer vblock
{
	vec4 velocities[]; 
};

void main()
{
	int index = int(gl_GlobalInvocationID);
	vec4 position = positions[index];
	vec4 velocity = velocities[index];
	position.xyz += dt * velocity.xyz;
	positions[index] = position;
};
