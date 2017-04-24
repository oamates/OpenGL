#version 330 core

in vec4 point_color;
out vec4 color;

void main()
{
    vec2 r = 2.0f * (gl_PointCoord - vec2(0.5f, 0.5f));
	if (dot(r, r) > 1.0f) discard;
    color = point_color;
//	color.w = (dot(r, r) < 0.5f) ? 1.0f : 0.0f;

};



