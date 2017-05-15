#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject
{
    mat4 projection_view_matrix;
    vec4 camera_ws4;
    vec4 light_ws4;
};

layout(binding = 1) uniform sampler2D model_tex;

layout(location = 0) in vec3 position_ws;
layout(location = 1) in vec3 normal_ws;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec4 FragmentColor;




void main()
{
    vec3 camera_ws = vec3(camera_ws4);
    vec3 light_ws = vec3(light_ws4);

    vec3 view = camera_ws - position_ws;
    vec3 light = light_ws - position_ws;

    float distance = length(light);

    vec3 v = normalize(view);
    vec3 l = light / distance;
    vec3 n = normalize(normal_ws);

    vec3 diffuse_color = texture(model_tex, uv).rgb;
    vec3 ambient_color = 0.15f * diffuse_color;
    vec3 specular_color = vec3(1.0f);

    vec3 color = ambient_color;

    float cos_theta = dot(n, l);
    if (cos_theta > 0.0f) 
    {
        color += cos_theta * diffuse_color;

        vec3 h = normalize(l + v);
        float cos_alpha = max(dot(h, n), 0.0f);
        float exponent = 40.0;

        color += pow(cos_alpha, exponent) * specular_color;
    }

    FragmentColor = vec4(color, 1.0f);
}