#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 12) out;

in vec3 vertNormal[];
in float vd[];

uniform mat4 projection_matrix;

out float geomAlpha;

void main()
{
    for (int v = 0; v != 3; ++v)
    {
        int a = v, b = (v + 1) % 3, c = (v + 2) % 3;
        vec4 pa = gl_in[a].gl_Position;
        vec4 pb = gl_in[b].gl_Position;
        vec4 pc = gl_in[c].gl_Position;
        vec4 px, py;
        vec3 na = vertNormal[a];
        vec3 nb = vertNormal[b];
        vec3 nc = vertNormal[c];
        vec3 nx, ny;

        if (vd[a] == 0.0 && vd[b] == 0.0)
        {
            px = pa;
            nx = na;
            py = pb;
            ny = nb;
        }
        else if (vd[a] > 0.0 && vd[b] < 0.0)
        {
            float x = vd[a] / (vd[a] - vd[b]);
            float y;
            px = mix(pa, pb, x);
            nx = mix(na, nb, x);
            if (vd[c] < 0.0)
            {
                y = vd[a] / (vd[a] - vd[c]);
                py = mix(pa, pc, y);
                ny = mix(na, nc, y);
            }
            else
            {
                y = vd[c] / (vd[c] - vd[b]);
                py = mix(pc, pb, y);
                ny = mix(nc, nb, y);
            }
        }
        else continue;

        vec4 gx1 = vec4(px.xyz, 1.0);
        vec4 gy1 = vec4(py.xyz, 1.0);
        vec4 gx2 = vec4(px.xyz + nx * 0.3, 1.0);
        vec4 gy2 = vec4(py.xyz + ny * 0.3, 1.0);

        gl_Position = projection_matrix * gy1;
        geomAlpha = 1.0;
        EmitVertex();
        gl_Position = projection_matrix * gx1;
        geomAlpha = 1.0;
        EmitVertex();
        gl_Position = projection_matrix * gy2;
        geomAlpha = 0.0;
        EmitVertex();
        gl_Position = projection_matrix * gx2;
        geomAlpha = 0.0;
        EmitVertex();
        EndPrimitive();
        break;
    }
}
