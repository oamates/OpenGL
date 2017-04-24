#version 330 core
layout (location = 0) in vec3 position_in;
layout (location = 1) in vec3 normal_in;
layout (location = 2) in vec2 uv_in;

out vec3 position_ws;
out vec3 normal_ws;
out vec2 uv;

uniform mat4 projection_matrix;
uniform mat4 view_matrix;
uniform mat4 model_matrix;

uniform int inverse_normals;

void main()
{
    vec4 position_ws4 = model_matrix * vec4(position_in, 1.0f);
    gl_Position = projection_matrix * view_matrix * position_ws4;
    position_ws = vec3(position_ws4);   
    uv = uv_in;
    
    vec3 n = (inverse_normals == 0) ? normal_in : -normal_in;
    
    mat3 normal_matrix = transpose(inverse(mat3(model_matrix)));
    normal_ws = normalize(normal_matrix * n);
}