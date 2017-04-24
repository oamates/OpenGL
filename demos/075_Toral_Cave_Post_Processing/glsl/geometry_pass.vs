#version 330 core                                                                        
                                                                                    
layout (location = 0) in vec3 position_in;

uniform mat4 projection_view_matrix;

void main()
{       
    gl_Position = projection_view_matrix * vec4(position_in, 1.0);
}