#version 330 core

in vec4 Position;
out float vertZOffs;

uniform int SampleCount;
uniform mat4 CameraMatrix;
uniform vec3 ViewZ;
uniform float Size;

void main()
{
	float hp = (SampleCount - 1) * 0.5;
	vertZOffs = (gl_InstanceID - hp) / hp;
	gl_Position = vec4(Position.xyz + ViewZ * vertZOffs * Size * 0.5, 1.0);
}
