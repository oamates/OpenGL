#version 330                                                                        

layout(binding = 0) uniform samplerCube cube_texture;

in vec3 colorf;
in vec3 texcoordf;

out vec4 FragmentColor;

uniform float time;
flat in int instanceID;

void main()
{
	FragmentColor = texture(cube_texture, texcoordf);	
}