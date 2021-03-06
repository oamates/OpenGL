#version 330 core

uniform sampler2D y_tex;
uniform sampler2D u_tex;
uniform sampler2D v_tex;

in vec2 v_coord;

layout(location = 0) out vec4 FragmentColor;

const vec3 R_cf = vec3(1.164383,  0.000000,  1.596027);
const vec3 G_cf = vec3(1.164383, -0.391762, -0.812968);
const vec3 B_cf = vec3(1.164383,  2.017232,  0.000000);
const vec3 offset = vec3(-0.0625, -0.5, -0.5);

void main()
{
    vec3 yuv = offset + vec3(texture(y_tex, v_coord).r,
                             texture(u_tex, v_coord).r,
                             texture(v_tex, v_coord).r);

    FragmentColor = vec4(dot(yuv, R_cf), dot(yuv, G_cf), dot(yuv, B_cf), 1.0);
}

