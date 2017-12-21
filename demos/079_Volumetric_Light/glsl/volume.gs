#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in float vertZOffs[];

uniform mat4 CameraMatrix, ProjectionMatrix;
uniform mat4 TexProjectionMatrix;
uniform vec3 ViewX, ViewY;
uniform float Size;

out vec4 geomTexCoord;

void main()
{
	float zo = vertZOffs[0];
	float yo[2] = float[2](-1.0, 1.0);
	float xo[2] = float[2](-1.0, 1.0);
	for(int j = 0; j != 2; ++j)
    	for(int i = 0; i != 2; ++i)
	    {
		    vec4 v = vec4(gl_in[0].gl_Position.xyz + ViewX * xo[i] * Size + ViewY * yo[j] * Size, 1.0);
    		geomTexCoord = TexProjectionMatrix * v;
	    	gl_Position = ProjectionMatrix * CameraMatrix * v;
		    EmitVertex();
    	}
	EndPrimitive();
}
