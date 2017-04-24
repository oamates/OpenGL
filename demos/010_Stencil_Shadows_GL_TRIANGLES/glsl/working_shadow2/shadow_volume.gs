#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 10) out;

in vec3 position_ws[];
in vec3 normal_ws[];

uniform vec3 light_ws;
uniform mat4 projection_view_matrix;

const float bias = 0.0425f;

void main()
{

    vec3 A = position_ws[0];
    vec3 B = position_ws[1];
    vec3 C = position_ws[2];

    vec3 L = light_ws;

    vec3 lA = normalize(A - L);
    vec3 lB = normalize(B - L);
    vec3 lC = normalize(C - L);

    vec3 nA = normalize(normal_ws[0]);
    vec3 nB = normalize(normal_ws[1]);
    vec3 nC = normalize(normal_ws[2]);

    float dotA = dot(lA, nA);
    float dotB = dot(lB, nB);
    float dotC = dot(lC, nC);

    vec4 vAl = projection_view_matrix * vec4(A + bias * lA, 1.0f);
    vec4 vBl = projection_view_matrix * vec4(B + bias * lB, 1.0f);
    vec4 vCl = projection_view_matrix * vec4(C + bias * lC, 1.0f);

    vec4 vAi = projection_view_matrix * vec4(lA, 0.0f);
    vec4 vBi = projection_view_matrix * vec4(lB, 0.0f);
    vec4 vCi = projection_view_matrix * vec4(lC, 0.0f);    

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
                vec3 lA_C  = normalize((A * dotC - C * dotA) - L * (dotC - dotA));
                vec3 lB_C  = normalize((B * dotC - C * dotB) - L * (dotC - dotB));
                vec4 vA_Cl = projection_view_matrix * vec4((A * dotC - C * dotA) + bias * lA_C * (dotC - dotA), dotC - dotA);
                vec4 vB_Cl = projection_view_matrix * vec4((B * dotC - C * dotB) + bias * lB_C * (dotC - dotB), dotC - dotB);
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
                vec3 lC_B  = normalize((C * dotB - B * dotC) - L * (dotB - dotC));
                vec3 lA_B  = normalize((A * dotB - B * dotA) - L * (dotB - dotA));
                vec4 vC_Bl = projection_view_matrix * vec4((C * dotB - B * dotC) + bias * lC_B * (dotB - dotC), dotB - dotC);
                vec4 vA_Bl = projection_view_matrix * vec4((A * dotB - B * dotA) + bias * lA_B * (dotB - dotA), dotB - dotA);
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
                vec3 lA_B  = normalize((A * dotB - B * dotA) - L * (dotB - dotA));
                vec3 lA_C  = normalize((A * dotC - C * dotA) - L * (dotC - dotA));
                vec4 vA_Bl = projection_view_matrix * vec4((A * dotB - B * dotA) + bias * lA_B * (dotB - dotA), dotB - dotA);         
                vec4 vA_Cl = projection_view_matrix * vec4((A * dotC - C * dotA) + bias * lA_C * (dotC - dotA), dotC - dotA);         
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
                vec3 lB_A  = normalize((B * dotA - A * dotB) - L * (dotA - dotB));
                vec3 lC_A  = normalize((C * dotA - A * dotC) - L * (dotA - dotC));
                vec4 vB_Al = projection_view_matrix * vec4((B * dotA - A * dotB) + bias * lB_A * (dotA - dotB), dotA - dotB);
                vec4 vC_Al = projection_view_matrix * vec4((C * dotA - A * dotC) + bias * lC_A * (dotA - dotC), dotA - dotC);
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
                vec3 lB_C  = normalize((B * dotC - C * dotB) - L * (dotC - dotB));
                vec3 lB_A  = normalize((B * dotA - A * dotB) - L * (dotA - dotB));
                vec4 vB_Cl = projection_view_matrix * vec4((B * dotC - C * dotB) + bias * lB_C * (dotC - dotB), dotC - dotB);         
                vec4 vB_Al = projection_view_matrix * vec4((B * dotA - A * dotB) + bias * lB_A * (dotA - dotB), dotA - dotB);         
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
                vec3 lC_A  = normalize((C * dotA - A * dotC) - L * (dotA - dotC));
                vec3 lC_B  = normalize((C * dotB - B * dotC) - L * (dotB - dotC));
                vec4 vC_Al = projection_view_matrix * vec4((C * dotA - A * dotC) + bias * lC_A * (dotA - dotC), dotA - dotC);         
                vec4 vC_Bl = projection_view_matrix * vec4((C * dotB - B * dotC) + bias * lC_B * (dotB - dotC), dotB - dotC);         
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