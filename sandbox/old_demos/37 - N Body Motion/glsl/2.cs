#version 430 core
layout(local_size_x = 256) in;

layout(location = 0) uniform float dt;

layout(std430, binding=0) buffer pblock
{
	vec4 positions[];
};

layout(std430, binding=1) buffer vblock
{
	vec4 velocities[]; 
};

shared vec4 tmp[gl_WorkGroupSize.x];

void main() 
{
	int N = int(gl_NumWorkGroups.x * gl_WorkGroupSize.x);
	int index = int(gl_GlobalInvocationID);
	vec3 position = positions[index].xyz;
	vec3 velocity = velocities[index].xyz;
	vec3 acceleration = vec3(0.0f, 0.0f, 0.0f);
	for (int tile = 0; tile < N; tile += int(gl_WorkGroupSize.x)) 
	{
		tmp[gl_LocalInvocationIndex] = positions[tile + int(gl_LocalInvocationIndex)];
	    groupMemoryBarrier();
	    barrier();
		for(int i = 0; i < gl_WorkGroupSize.x; ++i) 
		{
        	vec3 other = tmp[i].xyz;
	        vec3 diff = position - other;
    	    float invdist = 1.0f / (length(diff) + 0.001f);
        	acceleration -= diff * 0.1f * invdist * invdist * invdist;
		};
	    groupMemoryBarrier();
	    barrier();
	};
	velocities[index] = vec4(velocity+dt*acceleration, 0.0f);
};
