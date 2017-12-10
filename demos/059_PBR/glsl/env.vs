#version 330 core

layout (location = 0) in vec3 position_in;

uniform mat4 projection_matrix;
uniform mat3 view_matrix;

out vec3 position_ws;

void main()
{
    position_ws = position_in;
	vec4 position_clip = projection_matrix * vec4(view_matrix * position_in, 1.0f);

    //==========================================================================================================================================================
    // Set fragment depth to projective infinity
    //==========================================================================================================================================================    
	gl_Position = position_clip.xyww;
}