#version 330 core

in vec4 Position;

uniform mat4 CameraMatrix, ProjectionMatrix;
uniform mat4 TexProjectionMatrix;

out vec2 vertChecker;
out vec4 vertTexCoord;

void main()
{
	gl_Position = ProjectionMatrix * CameraMatrix * Position;
	vertTexCoord = TexProjectionMatrix * Position;
	vertChecker = Position.xz;
}