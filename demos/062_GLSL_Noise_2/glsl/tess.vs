#version 420 core

layout(location = 0) in vec2 lattice_point_in;

out vec2 lattice_point;

void main()
{
    lattice_point = lattice_point_in;
}
