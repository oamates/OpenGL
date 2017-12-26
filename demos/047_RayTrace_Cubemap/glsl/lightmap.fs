#version 330 core

in vec3 position_ws;

uniform vec3 light_ws;
uniform vec3 sphere_positions[16];

out vec4 FragmentColor;

void main()
{
    float d_max = 0.0;
    vec3 L = light_ws;
    vec3 R = normalize(position_ws - light_ws);

    for(int i = 0; i != 16; ++i)
    {
        vec3 S = sphere_positions[i];
        float a = dot(R, R);
        float b = 2 * dot(L - S, R);
        float c = dot(L - S, L - S) - 1.0;
        float d = b * b - 4 * a * c;
        if (d_max < d)
            d_max = d;
    }

    float lt = exp(-d_max * 0.8);
    FragmentColor = vec4(lt, lt, lt, 1.0);
}