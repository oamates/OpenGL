#version 330 core

in vec2 uv_in;

out vec2 uv;

void main()
{
   gl_Position = vec4(uv_in, 0.0f, 1.0f);
   uv = 0.5f + 0.5f * uv_in;
   uv.y = 1.0f - uv.y;
}
