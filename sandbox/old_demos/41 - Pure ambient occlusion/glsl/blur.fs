#version 330

in vec2 texture_coord;

out vec4 ssao_blurred;

layout(binding = 0) uniform sampler2D ssao_image;

const vec2 texture_size = vec2(1920.0f, 1080.0f);

const vec2 offset[] = 
{
	vec2(-1.5f, -1.5f), vec2(-1.5f, -0.5f), vec2(-1.5f,  0.5f), vec2(-1.5f,  1.5f), 
	vec2(-0.5f, -1.5f), vec2(-0.5f, -0.5f), vec2(-0.5f,  0.5f), vec2(-0.5f,  1.5f), 
	vec2( 0.5f, -1.5f), vec2( 0.5f, -0.5f), vec2( 0.5f,  0.5f), vec2( 0.5f,  1.5f), 
	vec2( 1.5f, -1.5f), vec2( 1.5f, -0.5f), vec2( 1.5f,  0.5f), vec2( 1.5f,  1.5f)
};

const float weight [] = 
{
	1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 3.0f, 3.0f, 1.0f,
	1.0f, 3.0f, 3.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f

};

void main()
{
    vec3 average_color = vec3(0.0f);

    for (int i = 0 ; i < 16 ; i++)
            average_color += weight[i] * texture(ssao_image, texture_coord + offset[i] / texture_size).xyz;

    ssao_blurred = vec4(average_color / 24.0f, 1.0f);
}