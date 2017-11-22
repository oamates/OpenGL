#version 330 core


uniform mat4 viewProjMatrix;

in vec2 vPosition;
out vec3 vertWorldPos;

void main( void )
{
    vertWorldPos = vec3(vPosition, 0);
    gl_Position = viewProjMatrix * vec4(vPosition, 0, 1);
}
