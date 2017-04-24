#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 144) out;

uniform int LIGHT_SOURCES;
uniform vec4 light_ws[16];

const float znear = 2.0f;

in vec4 position_ws[];

smooth out float z;
                  
void main()
{
	vec4 position;
	gl_Layer = 0;                                

	for (int l = 0; l < LIGHT_SOURCES; ++l)
	{
		vec4 light_src = light_ws[l];	

		for (int i = 0; i < 3; ++i)
		{
			position = position_ws[i] - light_src;
			gl_Position = vec4( position.z, position.y, -position.x - znear, -position.x);
			z = -position.x;
			EmitVertex();
		};
		EndPrimitive(); gl_Layer++;
	    
		for (int i = 0; i < 3; ++i)
		{
			position = position_ws[i] - light_src;
			gl_Position = vec4(-position.z, position.y,  position.x - znear,  position.x);
			z = position.x;
			EmitVertex();
		};
		EndPrimitive(); gl_Layer++;
	    
		for (int i = 0; i < 3; ++i)
		{
			position = position_ws[i] - light_src;
			gl_Position = vec4(-position.x, -position.z, -position.y - znear, -position.y);
			z = -position.y;
			EmitVertex();
		};
		EndPrimitive(); gl_Layer++;
	    
		for (int i = 0; i < 3; ++i)
		{
			position = position_ws[i] - light_src;
			gl_Position = vec4(-position.x,  position.z,  position.y - znear,  position.y);
			z = position.y;
			EmitVertex();
		};
		EndPrimitive(); gl_Layer++;
	    
		for (int i = 0; i < 3; ++i)
		{
			position = position_ws[i] - light_src;
			gl_Position = vec4(-position.x,  position.y, -position.z - znear, -position.z);
			z = -position.z;
			EmitVertex();
		};
		EndPrimitive(); gl_Layer++;
	    
		for (int i = 0; i < 3; ++i)
		{
			position = position_ws[i] - light_src;
			gl_Position = vec4( position.x,  position.y,  position.z - znear,  position.z);
			z = position.z;
			EmitVertex();
		};
		EndPrimitive(); gl_Layer++;
	};
};
