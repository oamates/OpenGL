#version 330 core

in vec4 point_color;
out vec4 color;

void main()
{
    vec2 r = 2.0f * (gl_PointCoord - vec2(0.5f, 0.5f));
    color = point_color * exp(-length(r));
//	color.a = 1.0; // - dot(r, r);
};

