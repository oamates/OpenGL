#version 330                                                                        
                                                                                    
layout (location = 0) in vec3 position_ms;                                             

uniform mat4 mvp_matrix;

void main()
{       
    gl_Position = mvp_matrix * vec4(position_ms, 1.0);
};