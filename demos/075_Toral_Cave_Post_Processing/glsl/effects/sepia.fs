#version 330 core

uniform sampler2D scene_texture;
in vec2 uv;
out vec4 FragmentColor;

const mat3 sepia_transform = mat3(vec3(0.393f, 0.769f, 0.189f),
                                  vec3(0.349f, 0.686f, 0.168f),
                                  vec3(0.272f, 0.534f, 0.131f));

void main ()
{
	FragmentColor = vec4(sepia_transform * texture(scene_texture, uv).rgb, 1.0f);
}
