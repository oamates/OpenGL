#version 430

layout (location = 0) in vec3 position_ms;

uniform mat4 model_matrix[343];

out vec4 position_ws;
                  
void main()
{
    position_ws = model_matrix[gl_InstanceID] * vec4(position_ms, 1.0f);
}
