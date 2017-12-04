#version 450

in vec3 normal_viewspace_f;
in vec3 normal_worldspace_f;
in vec3 vertexPosition_viewspace_f;
in vec3 vertexPosition_worldspace_f;
in vec3 eyePosition_worldspace_f;

layout(RGBA8) uniform image3D voxelImage;
uniform float sceneScale;

void main()
{
    ivec3 size = imageSize(voxelImage);
	ivec3 texCoord = ivec3((vertexPosition_worldspace_f / sceneScale + vec3(1)) / 2 * size);
    imageStore(voxelImage, ivec3(texCoord), vec4(0,0,0,0));
}