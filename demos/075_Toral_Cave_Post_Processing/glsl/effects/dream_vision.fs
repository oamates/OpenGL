#version 330 core                                                                        

uniform sampler2D scene_texture;

in vec2 uv;

out vec4 FragmentColor;

void main ()
{
	vec4 color = texture(scene_texture, uv) + 
				 texture(scene_texture, uv + 0.001f) +
				 texture(scene_texture, uv + 0.003f) +
				 texture(scene_texture, uv + 0.005f) +
				 texture(scene_texture, uv + 0.007f) +
				 texture(scene_texture, uv + 0.009f) +
				 texture(scene_texture, uv + 0.011f) +
				 texture(scene_texture, uv - 0.001f) +
				 texture(scene_texture, uv - 0.003f) +
				 texture(scene_texture, uv - 0.005f) +
				 texture(scene_texture, uv - 0.007f) +
				 texture(scene_texture, uv - 0.009f) +
				 texture(scene_texture, uv - 0.011f);
 
	color.rgb = vec3((color.r + color.g + color.b) / 3.0f);
	FragmentColor = color / 9.5f;
}
