#version 330 core

layout(location = 0) in mat4 frame;

out mat4 model_matrix;

void main()
{
    model_matrix = frame;
}


