#version 430 core

layout(binding = 0) uniform samplerCube skybox_texture;
const vec4 point_color = vec4(1.0f, 1.0f, 0.0f, 0.5f);
out vec4 color;
in vec3 point;


void main()
{
//	r = -0.5 + cos(time*0.4+1.5)*0.5;
//	g = -0.5 + cos(time*0.6)*sin(time*0.3)*0.35;
//	b = -0.5 + sin(time*0.2)*0.5;

    vec2 r = 2.0f * (gl_PointCoord - vec2(0.5f, 0.5f));
	vec3 c = 0.5 * vec3(texture(skybox_texture, point)) + vec3(0.5f, 0.5f, 0.0f);

    color = vec4(c, 1.0 - dot(r, r));
}

