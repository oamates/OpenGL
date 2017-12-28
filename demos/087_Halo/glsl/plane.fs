#version 330 core

in vec3 vertNormal;
in vec3 vertLight;

out vec4 FragmentColor;

void main()
{
    float l = sqrt(length(vertLight));
    float e = dot(vertNormal, normalize(vertLight));
    float d = l > 0.0 ? e / l : 0.0;
    float i = 0.2 + 2.5 * d;
    FragmentColor = vec4(0.8 * i, 0.7 * i, 0.4 * i, 1.0);
}
