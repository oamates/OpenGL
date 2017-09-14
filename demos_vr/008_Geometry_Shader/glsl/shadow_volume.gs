#version 430 core

layout (triangles_adjacency) in;                                    // six vertices in
layout (triangle_strip, max_vertices = 72) out;

in vec3 position_ws[];

uniform vec3 light_ws;

const int ovrEye_Count = 2;
uniform mat4 projection_view_matrix[ovrEye_Count];

const float lbias = 0.035625;
const float ibias = 10000.0125;

void main()
{
    vec3 e02 = position_ws[2] - position_ws[0];                     // vector AB
    vec3 e24 = position_ws[4] - position_ws[2];                     // vector BC
    vec3 e40 = position_ws[0] - position_ws[4];                     // vector CA

    vec3 l0 = normalize(position_ws[0] - light_ws);                 // from light to A
    vec3 normal = cross(e02, e24);

    if (dot(l0, normal) > 0) return;                                // handle only light facing triangles

    vec3 l2 = normalize(position_ws[2] - light_ws);                 // from light to B
    vec3 l4 = normalize(position_ws[4] - light_ws);                 // from light to C

    vec3 e01 = position_ws[1] - position_ws[0];
    vec3 e23 = position_ws[3] - position_ws[2];
    vec3 e45 = position_ws[5] - position_ws[4];

    // six vertices of the infinite prism for each viewport
    vec4 v0l[ovrEye_Count], v2l[ovrEye_Count], v4l[ovrEye_Count],
         v0i[ovrEye_Count], v2i[ovrEye_Count], v4i[ovrEye_Count];

    for (int e = 0; e < ovrEye_Count; ++e)
    {
        v0l[e] = projection_view_matrix[e] * vec4(position_ws[0] + lbias * l0, 1.0f);
        v2l[e] = projection_view_matrix[e] * vec4(position_ws[2] + lbias * l2, 1.0f);
        v4l[e] = projection_view_matrix[e] * vec4(position_ws[4] + lbias * l4, 1.0f);

        v0i[e] = projection_view_matrix[e] * vec4(l0, 0.0f);
        v2i[e] = projection_view_matrix[e] * vec4(l2, 0.0f);
        v4i[e] = projection_view_matrix[e] * vec4(l4, 0.0f);
    }

    for (int vp = 0; vp < ovrEye_Count; ++vp)
    {
        
        gl_Position = v0l[vp]; EmitVertex();
        gl_Position = v2l[vp]; EmitVertex();
        gl_Position = v4l[vp]; gl_ViewportIndex = vp; EmitVertex();
        EndPrimitive();
    }



    normal = cross(e01, e02);                                       // test the triangle adjacent to AB edge
    if ((dot(normal, l0) >= 0))
    {
        for (int vp = 0; vp < ovrEye_Count; ++vp)
        {
            
            gl_Position = v0l[vp]; EmitVertex();
            gl_Position = v0i[vp]; EmitVertex();
            gl_Position = v2l[vp]; EmitVertex();
            gl_Position = v2i[vp]; gl_ViewportIndex = vp; EmitVertex();
            EndPrimitive();
        }
    }

    normal = cross(e23, e24);                                       // test the triangle adjacent to BC edge
    if ((dot(normal, l2) >= 0))
    {
        for (int vp = 0; vp < ovrEye_Count; ++vp)
        {
            gl_Position = v2l[vp]; EmitVertex();
            gl_Position = v2i[vp]; EmitVertex();
            gl_Position = v4l[vp]; EmitVertex();
            gl_Position = v4i[vp]; gl_ViewportIndex = vp; EmitVertex();
            EndPrimitive();
        }
    }

    normal = cross(e45, e40);                                       // test the triangle adjacent to CA edge
    if ((dot(normal, l4) >= 0))
    {
        for (int vp = 0; vp < ovrEye_Count; ++vp)
        {
            
            gl_Position = v4l[vp]; EmitVertex();
            gl_Position = v4i[vp]; EmitVertex();
            gl_Position = v0l[vp]; EmitVertex();
            gl_Position = v0i[vp]; gl_ViewportIndex = vp; EmitVertex();
            EndPrimitive();
        }
    }
    
    for (int vp = 0; vp < ovrEye_Count; ++vp)                       // emit the projective triangle at infinity
    {
        gl_ViewportIndex = vp;
        gl_Position = v4i[vp]; EmitVertex();
        gl_Position = v2i[vp]; EmitVertex();
        gl_Position = v0i[vp]; EmitVertex();
        EndPrimitive();
    }
}