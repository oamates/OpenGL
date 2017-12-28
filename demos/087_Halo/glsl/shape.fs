#version 330 core

in vec3 vertNormal;
in vec3 vertViewNormal;
in vec3 vertLight;

uniform mat4 view_matrix;

out vec4 FragmentColor;

void main()
{
    float ltlen = sqrt(length(vertLight));
    float ltexp = dot(normalize(vertNormal), normalize(vertLight));
    float lview = dot(normalize(vertLight), normalize(vec3(view_matrix[0][2], view_matrix[1][2], view_matrix[2][2])));

    float depth = normalize(vertViewNormal).z;

    vec3 ftrefl  = vec3(0.9, 0.8, 0.7);
    vec3 scatter = vec3(0.9, 0.6, 0.1);
    vec3 bklt    = vec3(0.8, 0.6, 0.4);
    vec3 ambient = vec3(0.5, 0.4, 0.3);

    FragmentColor = vec4(
        pow(max(ltexp, 0.0), 8.0) * ftrefl +
        ( ltexp + 1.0) / ltlen * pow(depth, 2.0) * scatter +
        (-ltexp + 1.0) / ltlen * (1.0 - depth) * scatter +
        (-lview + 1.0) * 0.6 * (1.0 - abs(depth)) * bklt + 0.2 * ambient,
        1.0
    );
}