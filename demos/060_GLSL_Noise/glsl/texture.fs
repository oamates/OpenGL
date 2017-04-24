#version 330

uniform sampler2D noise_texture;
in vec2 v_texCoord2D;

out vec4 FragmentColor;

void main()
{
    FragmentColor = texture(noise_texture, v_texCoord2D);
}
