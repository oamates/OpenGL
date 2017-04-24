#version 400 core

out vec2 lattice_point;

void main()
{
    lattice_point = vec2(float(gl_InstanceID & 0x3F), float(gl_InstanceID >> 6)) - vec2(32);
}
