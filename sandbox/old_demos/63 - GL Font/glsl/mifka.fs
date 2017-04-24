#version 330                                                                        

layout(binding = 0) uniform sampler2D triangle_image0;
layout(binding = 1) uniform sampler2D triangle_image1;
layout(binding = 2) uniform sampler2D triangle_image2;
layout(binding = 3) uniform sampler2D triangle_image3;

in vec3 colorf;
in vec2 texcoordf;

out vec4 FragmentColor;


uniform float time;
flat in int instanceID;


vec4 texget(vec2 tc, int instance_id)
{
	int q = (int(72185 * sin(72185 * instance_id))) & 0x3; 
	if (q == 0) return texture2D(triangle_image0, tc);
	if (q == 1) return texture2D(triangle_image1, tc);
	if (q == 2) return texture2D(triangle_image2, tc);
	return texture2D(triangle_image3, tc);	
}


void main()
{
	vec4 v = texget(texcoordf, instanceID);

	float t = (time + 50 * instanceID);
	float cs = cos(t);
	float sn = sin(t);

	v = vec4(v.x * cs - v.z * sn, v.y, v.x * sn + v.z * cs, v.w);
	v = vec4(v.x * cs - v.y * sn, v.x * sn + v.y * cs, v.z, v.w);
	v = vec4(v.x, v.y * cs - v.z * sn, v.y * sn + v.z * cs, v.w);
	FragmentColor = abs(v);
}