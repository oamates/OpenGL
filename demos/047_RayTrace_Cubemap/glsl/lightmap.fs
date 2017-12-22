#version 330 core

in vec3 vertPosition;

uniform vec3 LightPosition;
uniform vec3 BallPositions[15];

out vec4 fragColor;

void main()
{
    float d_max = 0.0;
    vec3 L = LightPosition;
    vec3 R = normalize(vertPosition - LightPosition);
    for(int i = 0; i != 15; ++i)
    {
        vec3 S = BallPositions[i];
        float a = dot(R, R);
        float b = 2 * dot(L - S, R);
        float c = dot(L - S, L - S) - 1.0;
        float d = b * b - 4 * a * c;
        if (d_max < d)
            d_max = d;
    }
    float lt = exp(-d_max * 0.8);
    fragColor = vec4(lt, lt, lt, 1.0);
}