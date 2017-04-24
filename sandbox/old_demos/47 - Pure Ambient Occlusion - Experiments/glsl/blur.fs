#version 330

in vec2 texture_coord;

out float ssao_blurred;

layout(binding = 0) uniform sampler2D ssao_image;

const vec2 texture_size = vec2(1920.0f, 1080.0f);

void main()
{
    ssao_blurred = texture(ssao_image, texture_coord).x;
//    ssao_blurred = (texture(ssao_image, texture_coord + vec2( 0.5f, 0.5f) / texture_size).x + 
//					texture(ssao_image, texture_coord + vec2(-0.5f, 0.5f) / texture_size).x + 
//					texture(ssao_image, texture_coord + vec2( 0.5f,-0.5f) / texture_size).x + 
//					texture(ssao_image, texture_coord + vec2(-0.5f,-0.5f) / texture_size).x ) / 4.0f;
}