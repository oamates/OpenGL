#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 8) out;

in float dp[];
in vec4 vl[];
in vec4 vi[];

vec4 interpolate_l(int i, int j)
{
    return (vl[i] * dp[j] - vl[j] * dp[i]) / (dp[j] - dp[i]);
}

vec4 interpolate_i(int i, int j)
{
    return (vi[i] * dp[j] - vi[j] * dp[i]) / (dp[j] - dp[i]);
}


// {i, j, k} is a right triple : i is lit, j and k are in shadow
void emit6(int i, int j, int k)
{
    gl_Position = vl[i];               EmitVertex();
    gl_Position = interpolate_l(j, i); EmitVertex();
    gl_Position = interpolate_l(k, i); EmitVertex();
    gl_Position = interpolate_i(j, i); EmitVertex();
    gl_Position = interpolate_i(k, i); EmitVertex();
    gl_Position = vi[i];               EmitVertex();
    EndPrimitive();
}

// {i, j, k} is a right triple : i and j are lit, k is in shadow
void emit8(int i, int j, int k)
{
    gl_Position = vl[i];               EmitVertex();
    gl_Position = vl[j];               EmitVertex();
    gl_Position = interpolate_l(k, i); EmitVertex();
    gl_Position = interpolate_l(k, j); EmitVertex();
    gl_Position = interpolate_i(k, i); EmitVertex();
    gl_Position = interpolate_i(k, j); EmitVertex();
    gl_Position = vi[i];               EmitVertex();
    gl_Position = vi[j];               EmitVertex();
    EndPrimitive();
}

void main()
{
    if (dp[0] < 0.0f)                       // dp < 0 ~ the vertex is lit, dp >= 0 ~ the vertex is in shadow                    
    {
        if (dp[1] < 0.0f)                
        {
            if (dp[2] < 0.0f)               // 0-, 1-, 2-             
            {
                gl_Position = vl[0]; EmitVertex();
                gl_Position = vl[1]; EmitVertex();
                gl_Position = vl[2]; EmitVertex();
                EndPrimitive();

                gl_Position = vi[0]; EmitVertex();
                gl_Position = vi[2]; EmitVertex();
                gl_Position = vi[1]; EmitVertex();
                EndPrimitive();
            }
            else                            // 0-, 1-, 2+
                emit8(0, 1, 2);
        }
        else
        {
            if (dp[2] < 0.0f)               // 2-, 0-, 1+,            
                emit8(2, 0, 1);
            else                            // 0-, 1+, 2+
                emit6(0, 1, 2);
        }
            
    }
    else
    {
        if (dp[1] < 0.0f)                
        {
            if (dp[2] < 0.0f)               // 1-, 2-, 0+           
                emit8(1, 2, 0);
            else                            // 1-, 2+, 0+
                emit6(1, 2, 0);
        }
        else
        {
            if (dp[2] < 0.0f)               // 2-, 0+, 1+
                emit6(2, 0, 1);
            else                            // 0+, 1+, 2+
                return;
        }
    }
}