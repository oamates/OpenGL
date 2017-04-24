#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

const float znear = 1.0f;

in vec4 position_ws[];

uniform vec4 light_ws;
                  
void main()
{
	vec4 position;

	for (int i = 0; i < 3; ++i)
	{
		gl_Layer = 0;                                
		position = position_ws[i] - light_ws;
		gl_Position = vec4( position.z, -position.y, -position.x - znear, -position.x);
		EmitVertex();
	};
	EndPrimitive();

	for (int i = 0; i < 3; ++i)
	{
		gl_Layer = 1;                                
		position = position_ws[i] - light_ws;
		gl_Position = vec4(-position.z, -position.y,  position.x - znear,  position.x);
		EmitVertex();
	};
	EndPrimitive();

	for (int i = 0; i < 3; ++i)
	{
		gl_Layer = 2;                                
		position = position_ws[i] - light_ws;
		gl_Position = vec4( position.x, -position.z, -position.y - znear, -position.y);
		EmitVertex();
	};
	EndPrimitive();

	for (int i = 0; i < 3; ++i)
	{
		gl_Layer = 3;                                
		position = position_ws[i] - light_ws;
		gl_Position = vec4( position.x,  position.z,  position.y - znear,  position.y);
		EmitVertex();
	};
	EndPrimitive();

	for (int i = 0; i < 3; ++i)
	{
		gl_Layer = 4;                                
		position = position_ws[i] - light_ws;
		gl_Position = vec4(-position.x, -position.y, -position.z - znear, -position.z);
		EmitVertex();
	};
	EndPrimitive();

	for (int i = 0; i < 3; ++i)
	{
		gl_Layer = 5;                                
		position = position_ws[i] - light_ws;
		gl_Position = vec4( position.x, -position.y,  position.z - znear,  position.z);
		EmitVertex();
	};
	EndPrimitive();
};
