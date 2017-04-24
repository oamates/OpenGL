#version 430 core

layout (std430, binding = 0) buffer shader_data
{
	vec3 point3d[];
};

uniform mat4 projection_view_matrix;
uniform float global_time;


const vec3 angular_velocity = 10.0 * vec3(-0.966918f, 2.879879f, -1.3287621f);

vec3 iterate(const vec3 v)
{

	float t = global_time / 5.0;
	vec3 vX = 201.0 * vec3( 2.7651451f - 2.25512433 * cos(2.13254725 * t),  1.7651451f + 7.26215775 * sin(1.5545376 * t),  4.7651451f + 6.16436599 * sin(2.4243539 * t));
	vec3 vY = 110.0 * vec3( 4.7675471f + 3.34234123 * sin(3.12543675 * t),  3.7651451f - 3.44436233 * cos(2.3245654 * t),  2.3467557f - 8.67463341 * sin(3.5323525 * t));
	vec3 vZ = -159.0 * vec3( 2.3454551f - 8.24648522 * sin(2.67546546 * t),  2.24356234 - 9.17243622 * cos(4.1345667 * t),  3.2354451f + 3.11346324 * cos(1.7545173 * t));
	return normalize(vX * (sin(v.x * angular_velocity))
		           + vY * (sin(v.y * angular_velocity))
		           + vZ * (sin(v.y * angular_velocity)));
}


out vec3 point;

void main()
{
	//vec3 
	point = point3d[gl_VertexID];
	
	float m = max(max(abs(point.x), abs(point.y)), abs(point.z));

	vec4 point4d = vec4(20.0f * point / m, 1.0f);
	gl_Position = projection_view_matrix * point4d;
	point3d[gl_VertexID] = iterate(point);
    gl_PointSize = 1;
}


