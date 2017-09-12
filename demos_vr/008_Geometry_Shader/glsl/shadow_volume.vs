#version 430 core

layout (location = 0) in vec3 position_in;                                             

uniform vec3 shift;
out vec3 position_ws;
                                                                                    
void main()                                                                         
{                                                                                   
    position_ws = shift + position_in;
}
