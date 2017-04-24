#version 330 core

in vec3 uvw;

out vec4 FragmentColor;

void main()
{
    vec3 q = abs(uvw);
    FragmentColor = vec4(vec3(1.0f) - q, 0.33f / (1.0f + 25.0f * q.x * q.y * q.z));
}