#version 430

layout (location = 0) in vec3 position_ms;

layout(binding = 0) uniform modelpositions
{
	vec4 model_positions[1000];
};

out vec4 position_ws;
                  
void main()
{
    position_ws = model_positions[gl_InstanceID] + vec4(position_ms, 1.0f);
}
