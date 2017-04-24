#version 330 core
layout (location = 0) in vec3 position_in;
layout (location = 1) in vec3 normal_in;

out vec3 position_cs;
out vec3 normal_cs;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main()
{
    mat4 model_view_matrix = view_matrix * model_matrix;
    vec4 position_cs4 = model_view_matrix * vec4(position_in, 1.0f);
    position_cs = vec3(position_cs4); 
    gl_Position = projection_matrix * position_cs4;
    
    mat3 normal_matrix = transpose(inverse(mat3(model_view_matrix)));
    normal_cs = normal_matrix * normal_in;
}