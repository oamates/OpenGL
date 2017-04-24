#version 330 core

const vec4 point_color = vec4(1.0f, 1.0f, 0.0f, 0.5f);
out vec4 color;

void main()
{
    vec2 r = 2.0f * (gl_PointCoord - vec2(0.5f, 0.5f));
    color = vec4(vec3(point_color), 1.0 - dot(r, r));
};


