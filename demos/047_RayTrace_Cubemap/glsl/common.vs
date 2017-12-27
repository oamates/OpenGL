#version 330 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;
layout(location = 2) in vec3 tangent_x_in;
layout(location = 3) in vec3 tangent_y_in;
layout(location = 4) in vec2 uv_in;

uniform mat4 model_matrix;
uniform vec3 camera_ws;
uniform vec3 light_ws;

out gl_PerVertex
{
   vec4 gl_Position;
};

out vec3 vertNormal;
out vec3 vertTangent;
out vec3 vertBitangent;
out vec3 vertLightDir;
out vec3 vertViewDir;
out vec2 vertTexCoord;

void main()
{
    gl_Position = model_matrix * vec4(position_in, 1.0f);

    vertLightDir = light_ws - gl_Position.xyz;
    vertViewDir = camera_ws - gl_Position.xyz;
    vertNormal =    (model_matrix * vec4(normal_in,  0.0f)).xyz;
    vertTangent =   (model_matrix * vec4(tangent_x_in, 0.0f)).xyz;
    vertBitangent = (model_matrix * vec4(tangent_y_in, 0.0f)).xyz;
    vertTexCoord = uv_in;
}
