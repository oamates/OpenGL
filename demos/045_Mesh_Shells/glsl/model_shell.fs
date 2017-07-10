#version 330 core

in vec3 position_ws;
in vec3 normal_ws;
in float defect;
in vec3 manifold_mark;

uniform vec3 camera_ws;
uniform vec3 light_ws;

out vec4 FragmentColor;

void main()
{

    if (length(manifold_mark) > 0.0125)
    {
        FragmentColor = vec4(manifold_mark, 1.0f);
        return;
    }


    vec3 n = normalize(normal_ws);

    vec3 view = camera_ws - position_ws;
    float view_distance = length(view);
    vec3 v = view / view_distance;
    float distance_factor = 1.0 / (1.0 + 0.25 * view_distance);

    vec3 light = light_ws - position_ws;
    vec3 l = normalize(light);

    float cos_theta = 0.5 + 0.5 * dot(n, l);

    vec3 DEFECTIVE_COLOR = vec3(1.0, 0.0, 0.0);
    vec3 NORMAL_COLOR = vec3(0.35, 0.35, 0.0);

    float q = pow(defect, 4.0);

    vec3 color = mix(DEFECTIVE_COLOR, NORMAL_COLOR, q);
    vec3 ambient = 0.175 * color;
    vec3 diffuse = color;

    float diffuse_factor = cos_theta;
    vec3 h = normalize(l + v);
    float cos_alpha = max(dot(h, n), 0.0f);
    float specular_factor = 0.125 * pow(cos_alpha, 40.0);
    vec3 c = ambient + distance_factor * (diffuse_factor * diffuse + vec3(specular_factor));

    FragmentColor = vec4(c, 1.0f);
}
