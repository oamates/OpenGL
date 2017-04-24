#version 330 core

layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;

in vec3 position_ws[];

uniform mat4 projection_view_matrix;
uniform vec3 camera_ws;

const float size = 0.05f;

out vec2 uv;

void main()
{
    vec3 position_ws0 = position_ws[0];
    vec3 view_dir = normalize(camera_ws - position_ws0);                                    
    vec3 up = vec3(0.0, 1.0, 0.0);                                                  
    vec3 right = cross(view_dir, up) * size;                              
                                                                                    
    position_ws0 -= right;                                                                   
    gl_Position = projection_view_matrix * vec4(position_ws0, 1.0f);                                             
    uv = vec2(0.0, 0.0);                                                      
    EmitVertex();                                                                   
                                                                                    
    position_ws0.y += size;                                                        
    gl_Position = projection_view_matrix * vec4(position_ws0, 1.0f);                                             
    uv = vec2(0.0, 1.0);                                                      
    EmitVertex();                                                                   
                                                                                    
    position_ws0.y -= size;                                                        
    position_ws0 += right;                                                                   
    gl_Position = projection_view_matrix * vec4(position_ws0, 1.0f);                                             
    uv = vec2(1.0, 0.0);                                                      
    EmitVertex();                                                                   
                                                                                    
    position_ws0.y += size;                                                        
    gl_Position = projection_view_matrix * vec4(position_ws0, 1.0f);                                             
    uv = vec2(1.0, 1.0);                                                      
    EmitVertex();                                                             
}                                                                                   
