#version 330

out vec4 FragmentColor;

void main()
{
    float n = 0.0;
    FragmentColor = vec4(0.5 + 0.5 * vec3(n, n, n), 1.0);
}
