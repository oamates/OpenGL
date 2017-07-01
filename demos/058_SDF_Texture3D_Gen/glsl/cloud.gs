#version 330 core

layout (triangles) in;
layout (points, max_vertices = 24) out; 

in vec3 position_ws[];
 
uniform float sigma;

out vec3 internal_cloud;
out vec3 external_cloud;


void main()
{
    vec3 A = position_ws[0];
    vec3 B = position_ws[1];
    vec3 C = position_ws[2];

    vec3 AB = B - A;
    vec3 BC = C - B; 
    vec3 CA = C - B; 

    float lAB = length(AB);
    float lBC = length(BC);
    float lCA = length(CA);

    //=========================================================
    // determine the number of tesselation points on each edge
    //=========================================================

    int mAB = ...
    int mBC = ...
    int mCD = ...

    for (int i = 0; i < 3; i++)
    {
        internal_cloud = 

    mat4 frame = model_matrix[0];
    mat4 simplex_ws = frame * simplex_ms;  

    vec4 shift = frame[3];
    vec4 view_point = view_matrix[3];

    vec4 vertices[] = vec4[]
    (
        shift + simplex_ws[0], shift + simplex_ws[1], shift + simplex_ws[2], shift + simplex_ws[3],
        shift - simplex_ws[0], shift - simplex_ws[1], shift - simplex_ws[2], shift - simplex_ws[3]
    );

    vec4 normals[6]    = vec4[](-frame[2], frame[2],-frame[0], frame[0],-frame[1], frame[1]);
    vec4 tangents_x[6] = vec4[]( frame[1], frame[0], frame[2], frame[1], frame[0], frame[2]);
    vec4 tangents_y[6] = vec4[]( frame[0], frame[1], frame[1], frame[2], frame[2], frame[0]);

    int index = 0;    

    vec4 c = normalize(shift);
    for (int i = 0; i < 6; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            color = abs(c);
            view_direction = vertices[faces[index]] - view_point;
            gl_Position = projection_matrix * view_matrix * vertices[faces[index]];
            texture_coord = square[j];
            normal = normals[i];
            tangent_x = tangents_x[i];
            tangent_y = tangents_y[i];
            EmitVertex();
            ++index;
        }
        EndPrimitive();
    }
}