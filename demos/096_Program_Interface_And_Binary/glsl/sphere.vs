#version 330 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;

uniform vec4 spheres[128];
uniform mat4 projection_view_matrix;
uniform float time;

out vec3 color;

vec2 hash(vec2 n)
    { return fract(cos(n) * 753.5453123); }

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
	vec2 h = hash(vec2(gl_InstanceID, gl_VertexID));
    vec3 center = spheres[gl_InstanceID].xyz;
    float radius = spheres[gl_InstanceID].w;
    float theta = h.x + (1.5 + 1.5 * h.y) * time;
    float sn = sin(theta);
    float cs = cos(theta);
    float q = pow(abs(h.x * sn + h.y * cs), -1.25);

    vec3 position_ws = center + radius * q * (sn * position_in + cs * normal_in);

    color = hsv2rgb(vec3(h.x, 0.85f, exp(-0.0625 * q)));
    gl_Position = projection_view_matrix * vec4(position_ws, 1.0f);
}