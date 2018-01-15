#version 330 core

in vec3 ray;

uniform sampler2D hdr_tex;

out vec4 FragmentColor;

const vec2 inv_2pi_pi = vec2(0.15915494309189533576888376337251, 0.31830988618379067153776752674503);

void main()
{
    vec3 r = normalize(ray);
    vec2 uv = 0.5 + inv_2pi_pi * vec2(atan(r.x, r.y), asin(r.z));
    FragmentColor = texture(hdr_tex, uv);
}

