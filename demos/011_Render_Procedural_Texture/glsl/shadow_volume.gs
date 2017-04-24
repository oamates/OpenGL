#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 8) out;

invariant in vec3 position_ws[];
invariant in vec3 normal_ws[];
invariant in float dp[];
invariant in vec4 vl[];
invariant in vec4 vi[];

uniform vec3 light_ws;
uniform mat4 projection_view_matrix;

const float bias = 0.01425f;

//==============================================================================================================================================================
// dp[i0] < 0, dp[i1] < 0, dp[i2] >= 0, (i0, i1, i2) is the right triple
//==============================================================================================================================================================
void Emit8(int i0, int i1, int i2)              
{
    vec3 v02   = (dp[i2] * position_ws[i0] - dp[i0] * position_ws[i2]) / (dp[i2] - dp[i0]);
    vec3 v12   = (dp[i2] * position_ws[i1] - dp[i1] * position_ws[i2]) / (dp[i2] - dp[i1]);
    vec3 l02  = normalize(v02 - light_ws);
    vec3 l12  = normalize(v12 - light_ws);
    vec4 v02l = projection_view_matrix * vec4(v02 + bias * l02, 1.0f);
    vec4 v12l = projection_view_matrix * vec4(v12 + bias * l12, 1.0f);
    vec4 v02i = projection_view_matrix * vec4(l02, 0.0f);              
    vec4 v12i = projection_view_matrix * vec4(l12, 0.0f);              

    gl_Position = vl[i0]; EmitVertex();
    gl_Position = vl[i1]; EmitVertex();
    gl_Position = v02l;   EmitVertex();
    gl_Position = v12l;   EmitVertex();
    gl_Position = v02i;   EmitVertex();
    gl_Position = v12i;   EmitVertex();
    gl_Position = vi[i0]; EmitVertex();
    gl_Position = vi[i1]; EmitVertex();
}

//==============================================================================================================================================================
// dp[i0] < 0, dp[i1] >= 0, dp[i2] >= 0, (i0, i1, i2) is the right triple
//==============================================================================================================================================================
void Emit6(int i0, int i1, int i2)
{
    vec3 v01  = (dp[i1] * position_ws[i0] - dp[i0] * position_ws[i1]) / (dp[i1] - dp[i0]);
    vec3 v02  = (dp[i2] * position_ws[i0] - dp[i0] * position_ws[i2]) / (dp[i2] - dp[i0]);
    vec3 l01  = normalize(v01 - light_ws);
    vec3 l02  = normalize(v02 - light_ws);
    vec4 v01l = projection_view_matrix * vec4(v01 + bias * l01, 1.0f);         
    vec4 v02l = projection_view_matrix * vec4(v02 + bias * l02, 1.0f);         
    vec4 v01i = projection_view_matrix * vec4(l01, 0.0f);                       
    vec4 v02i = projection_view_matrix * vec4(l02, 0.0f);                       
        
    gl_Position = vl[i0]; EmitVertex();
    gl_Position = v01l;   EmitVertex();
    gl_Position = v02l;   EmitVertex();
    gl_Position = v01i;   EmitVertex();
    gl_Position = v02i;   EmitVertex();
    gl_Position = vi[i0]; EmitVertex();
}

void main()
{
    if (dp[0] < 0.0f)                // - means the vertex is lit, + means the vertex is in shadow                    
    {
        if (dp[1] < 0.0f)                
        {
            if (dp[2] < 0.0f)        // A-, B-, C-             
            {
                gl_Position = vl[0]; EmitVertex();
                gl_Position = vl[1]; EmitVertex();
                gl_Position = vl[2]; EmitVertex();
                EndPrimitive();
                gl_Position = vi[2]; EmitVertex();
                gl_Position = vi[1]; EmitVertex();
                gl_Position = vi[0]; EmitVertex();
            }
            else                     // A-, B-, C+
                Emit8(0, 1, 2);
        }
        else
        {
            if (dp[2] < 0.0f)        // C-, A-, B+,            
                Emit8(2, 0, 1);
            else                     // A-, B+, C+
                Emit6(0, 1, 2);
        }
            
    }
    else
    {
        if (dp[1] < 0.0f)                
        {
            if (dp[2] < 0.0f)        // B-, C-, A+
                Emit8(1, 2, 0);           
            else                    // B-, C+, A+
                Emit6(1, 2, 0);
        }
        else
        {
            if (dp[2] < 0.0f)        // C-, A+, B+
                Emit6(2, 0, 1);            
            else                    // A+, B+, C+
                return;
        }
    }
}