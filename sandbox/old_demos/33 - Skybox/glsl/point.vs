#version 330 core

layout(location = 0) in mat4 frame;  // incoming vertices


uniform vec3 shift;

uniform mat4 projection_view_matrix;
uniform float global_time;

out vec4 point_color;  

void main()
{
    float angle = frame[3][3] * global_time;
    float s = sin(angle);
    float c = cos(angle);
	vec4 q = (c * frame[0] + s * frame[1]) / (c + s);
    vec4 rotated_point = q + vec4(shift, 0.0);
	gl_Position = projection_view_matrix * rotated_point;
	point_color = frame[3];

//	float r = length(q);

    gl_PointSize = 5;
}


