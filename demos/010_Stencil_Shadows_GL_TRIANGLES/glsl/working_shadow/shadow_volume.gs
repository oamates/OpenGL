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

const float bias = 0.0425f;

void main()
{

    vec3 A = position_ws[0];
    vec3 B = position_ws[1];
    vec3 C = position_ws[2];

    vec3 L = light_ws;

    vec3 nA = normal_ws[0];
    vec3 nB = normal_ws[1];
    vec3 nC = normal_ws[2];

    float dotA = dp[0];
    float dotB = dp[1];
    float dotC = dp[2];

    vec4 vAl = vl[0];
    vec4 vBl = vl[1];
    vec4 vCl = vl[2];

    vec4 vAi = vi[0];
    vec4 vBi = vi[1];
    vec4 vCi = vi[2];

    if (dotA < 0.0f)                // - means the vertex is lit, + means the vertex is in shadow                    
    {
        if (dotB < 0.0f)                
        {
            if (dotC < 0.0f)        // A-, B-, C-             
            {
                gl_Position = vAl; EmitVertex();
                gl_Position = vBl; EmitVertex();
                gl_Position = vCl; EmitVertex();
                EndPrimitive();
                gl_Position = vCi; EmitVertex();
                gl_Position = vBi; EmitVertex();
                gl_Position = vAi; EmitVertex();
                EndPrimitive();
            }
            else                    // A-, B-, C+
            {
                vec3 A_C   = (A * dotC - C * dotA) / (dotC - dotA);
                vec3 B_C   = (B * dotC - C * dotB) / (dotC - dotB);
                vec3 lA_C  = normalize(A_C - L);
                vec3 lB_C  = normalize(B_C - L);
                vec4 vA_Cl = projection_view_matrix * vec4(A_C + bias * lA_C, 1.0f);
                vec4 vB_Cl = projection_view_matrix * vec4(B_C + bias * lB_C, 1.0f);
                vec4 vA_Ci = projection_view_matrix * vec4(lA_C, 0.0f);              
                vec4 vB_Ci = projection_view_matrix * vec4(lB_C, 0.0f);              

                gl_Position = vAl;   EmitVertex();
                gl_Position = vBl;   EmitVertex();
                gl_Position = vA_Cl; EmitVertex();
                gl_Position = vB_Cl; EmitVertex();
                gl_Position = vA_Ci; EmitVertex();
                gl_Position = vB_Ci; EmitVertex();
                gl_Position = vAi;   EmitVertex();
                gl_Position = vBi;   EmitVertex();
                EndPrimitive();
            }
            
        }
        else
        {
            if (dotC < 0.0f)        // C-, A-, B+,            
            {
                vec3  C_B  = (C * dotB - B * dotC) / (dotB - dotC);
                vec3  A_B  = (A * dotB - B * dotA) / (dotB - dotA);
                vec3 lC_B  = normalize(C_B - L);
                vec3 lA_B  = normalize(A_B - L);
                vec4 vC_Bl = projection_view_matrix * vec4(C_B + bias * lC_B, 1.0f);
                vec4 vA_Bl = projection_view_matrix * vec4(A_B + bias * lA_B, 1.0f);
                vec4 vC_Bi = projection_view_matrix * vec4(lC_B, 0.0f);              
                vec4 vA_Bi = projection_view_matrix * vec4(lA_B, 0.0f);              

                gl_Position = vCl;   EmitVertex();
                gl_Position = vAl;   EmitVertex();
                gl_Position = vC_Bl; EmitVertex();
                gl_Position = vA_Bl; EmitVertex();
                gl_Position = vC_Bi; EmitVertex();
                gl_Position = vA_Bi; EmitVertex();
                gl_Position = vCi;   EmitVertex();
                gl_Position = vAi;   EmitVertex();
                EndPrimitive();
            
            }
            else                    // A-, B+, C+
            {
                vec3  A_B  = (A * dotB - B * dotA) / (dotB - dotA);
                vec3  A_C  = (A * dotC - C * dotA) / (dotC - dotA);
                vec3 lA_B  = normalize(A_B - L);
                vec3 lA_C  = normalize(A_C - L);
                vec4 vA_Bl = projection_view_matrix * vec4(A_B + bias * lA_B, 1.0f);         
                vec4 vA_Cl = projection_view_matrix * vec4(A_C + bias * lA_C, 1.0f);         
                vec4 vA_Bi = projection_view_matrix * vec4(lA_B, 0.0f);                       
                vec4 vA_Ci = projection_view_matrix * vec4(lA_C, 0.0f);                       
                    
                gl_Position = vAl;   EmitVertex();
                gl_Position = vA_Bl; EmitVertex();
                gl_Position = vA_Cl; EmitVertex();
                gl_Position = vA_Bi; EmitVertex();
                gl_Position = vA_Ci; EmitVertex();
                gl_Position = vAi;   EmitVertex();
                EndPrimitive();

            }
        }
            
    }
    else
    {
        if (dotB < 0.0f)                
        {
            if (dotC < 0.0f)        // B-, C-, A+           
            {
                vec3  B_A  = (B * dotA - A * dotB) / (dotA - dotB);
                vec3  C_A  = (C * dotA - A * dotC) / (dotA - dotC);
                vec3 lB_A  = normalize(B_A - L);
                vec3 lC_A  = normalize(C_A - L);
                vec4 vB_Al = projection_view_matrix * vec4(B_A + bias * lB_A, 1.0f);
                vec4 vC_Al = projection_view_matrix * vec4(C_A + bias * lC_A, 1.0f);
                vec4 vB_Ai = projection_view_matrix * vec4(lB_A, 0.0f);              
                vec4 vC_Ai = projection_view_matrix * vec4(lC_A, 0.0f);              

                gl_Position = vBl;   EmitVertex();
                gl_Position = vCl;   EmitVertex();
                gl_Position = vB_Al; EmitVertex();
                gl_Position = vC_Al; EmitVertex();
                gl_Position = vB_Ai; EmitVertex();
                gl_Position = vC_Ai; EmitVertex();
                gl_Position = vBi;   EmitVertex();
                gl_Position = vCi;   EmitVertex();
                EndPrimitive();
            
            }
            else                    // B-, C+, A+
            {
                vec3  B_C  = (B * dotC - C * dotB) / (dotC - dotB);
                vec3  B_A  = (B * dotA - A * dotB) / (dotA - dotB);
                vec3 lB_C  = normalize(B_C - L);
                vec3 lB_A  = normalize(B_A - L);
                vec4 vB_Cl = projection_view_matrix * vec4(B_C + bias * lB_C, 1.0f);         
                vec4 vB_Al = projection_view_matrix * vec4(B_A + bias * lB_A, 1.0f);         
                vec4 vB_Ci = projection_view_matrix * vec4(lB_C, 0.0f);                       
                vec4 vB_Ai = projection_view_matrix * vec4(lB_A, 0.0f);                       
                    
                gl_Position = vBl;   EmitVertex();
                gl_Position = vB_Cl; EmitVertex();
                gl_Position = vB_Al; EmitVertex();
                gl_Position = vB_Ci; EmitVertex();
                gl_Position = vB_Ai; EmitVertex();
                gl_Position = vBi;   EmitVertex();
                EndPrimitive();
            }
            
        }
        else
        {
            if (dotC < 0.0f)        // C-, A+, B+            
            {
                vec3  C_A  = (C * dotA - A * dotC) / (dotA - dotC);
                vec3  C_B  = (C * dotB - B * dotC) / (dotB - dotC);
                vec3 lC_A  = normalize(C_A - L);
                vec3 lC_B  = normalize(C_B - L);
                vec4 vC_Al = projection_view_matrix * vec4(C_A + bias * lC_A, 1.0f);         
                vec4 vC_Bl = projection_view_matrix * vec4(C_B + bias * lC_B, 1.0f);         
                vec4 vC_Ai = projection_view_matrix * vec4(lC_A, 0.0f);                       
                vec4 vC_Bi = projection_view_matrix * vec4(lC_B, 0.0f);                       
                    
                gl_Position = vCl;   EmitVertex();
                gl_Position = vC_Al; EmitVertex();
                gl_Position = vC_Bl; EmitVertex();
                gl_Position = vC_Ai; EmitVertex();
                gl_Position = vC_Bi; EmitVertex();
                gl_Position = vCi;   EmitVertex();
                EndPrimitive();
            
            }
            else                    // A+, B+, C+
            {
                return;
            }
        }
    }
}