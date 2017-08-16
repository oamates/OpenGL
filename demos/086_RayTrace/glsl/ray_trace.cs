#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

//==============================================================================================================================================================
// Predefined compute shader inputs :: 
//  const uvec3 gl_WorkGroupSize      = uvec3(local_size_x, local_size_y, local_size_z)
//  in uvec3 gl_NumWorkGroups         ----- the number of work groups passed to the dispatch function
//  in uvec3 gl_WorkGroupID           ----- the current work group for this shader invocation
//  in uvec3 gl_LocalInvocationID     ----- the current invocation of the shader within the work group
//  in uvec3 gl_GlobalInvocationID    ----- unique identifier of this invocation of the compute shader among all invocations of this compute dispatch call
//==============================================================================================================================================================

layout (rgba8, binding = 0) uniform image2D scene_image;

uniform mat3 camera_matrix;
uniform vec3 camera_ws;
uniform vec2 focal_scale;
uniform vec2 resolution;
uniform sampler2D tb_tex;
uniform vec3 light_ws;

const vec4 fpar00[16] = vec4[]
(
    vec4(-1.0f, 0.0f,  5.0f, 1.51f),
    vec4( 3.0f, 1.0f, -3.1f, 1.37f),
    vec4(-1.0f, 0.0f,  5.0f, 1.51f),
    vec4( 3.0f, 1.0f, -3.1f, 1.37f),

    vec4(-1.0f, 0.0f,  5.0f, 1.51f),
    vec4( 3.0f, 1.0f, -3.1f, 1.37f),
    vec4(-1.0f, 0.0f,  5.0f, 1.51f),
    vec4( 3.0f, 1.0f, -3.1f, 1.37f),

    vec4(-1.0f, 0.0f,  5.0f, 1.51f),
    vec4( 3.0f, 1.0f, -3.1f, 1.37f),
    vec4(-1.0f, 0.0f,  5.0f, 1.51f),
    vec4( 3.0f, 1.0f, -3.1f, 1.37f),

    vec4(-1.0f, 0.0f,  5.0f, 1.51f),
    vec4( 3.0f, 1.0f, -3.1f, 1.37f),
    vec4(-1.0f, 0.0f,  5.0f, 1.51f),
    vec4( 3.0f, 1.0f, -3.1f, 1.37f)
);

bool intSphere(in vec4 sp, in vec3 ro, in vec3 rd, in float tm, out float t)
{
    bool  r = false;
    vec3  d = ro - sp.xyz;
    float b = dot(rd, d);
    float c = dot(d, d) - sp.w * sp.w;
    t = b * b - c;
    if(t > 0.0)
    {
        t = -b - sqrt(t);
        r = (t > 0.0) && (t < tm);
    }
    return r;
}

bool intCylinder(in vec4 sp, in vec3 ro, in vec3 rd, in float tm, out float t)
{
    bool r = false;
    vec3  d = ro - sp.xyz;
    float a = dot(rd.xz, rd.xz);
    float b = dot(rd.xz, d.xz);
    float c = dot(d.xz, d.xz) - sp.w * sp.w;
    t = b * b - a * c;
    if(t > 0.0)
    {
        t = (-b - sqrt(t) * sign(sp.w)) / a;
        r = (t > 0.0) && (t < tm);
    }
    return r;
}

bool intPlane(in vec4 pl, in vec3 ro, in vec3 rd, in float tm, out float t)
{
    t = -(dot(pl.xyz, ro) + pl.w) / dot(pl.xyz,rd);
    return (t > 0.0) && (t < tm);
}

vec3 calcnor(in vec4 ob,in vec4 ot,in vec3 po,out vec2 uv)
{
    vec3 no;

    if(ot.w > 2.5f)
    {
        no.xz = po.xz - ob.xz;
        no.y = 0.0;
        no = no / ob.w;
        uv = vec2(no.x, po.y + 1.0f);
    }
    else if(ot.w > 1.5f)
    {
        no = ob.xyz;
        uv = po.xz * 0.2f;
    }
    else
    {
        no = po - ob.xyz;
        no = no / ob.w;
        uv = no.xy;
    }

    return no;
}


float calc_inter(in vec3 ro,in vec3 rd,out vec4 ob, out vec4 co)
{
    float tm = 10000.0;
    float t;

    if(intSphere  (fpar00[0], ro, rd, tm, t)) { ob = fpar00[0]; co = fpar00[ 6]; tm = t; }
    if(intSphere  (fpar00[1], ro, rd, tm, t)) { ob = fpar00[1]; co = fpar00[ 7]; tm = t; }
    if(intCylinder(fpar00[2], ro, rd, tm, t)) { ob = fpar00[2]; co = fpar00[ 8]; tm = t; }
    if(intCylinder(fpar00[3], ro, rd, tm, t)) { ob = fpar00[3]; co = fpar00[ 9]; tm = t; }
    if(intPlane   (fpar00[4], ro, rd, tm, t)) { ob = fpar00[4]; co = fpar00[10]; tm = t; }
    if(intPlane   (fpar00[5], ro, rd, tm, t)) { ob = fpar00[5]; co = fpar00[11]; tm = t; }

    return tm;
}


bool calc_shadow(in vec3 ro,in vec3 rd,in float l)
{
    float t;

    bvec4 ss = bvec4(intSphere  (fpar00[0], ro, rd, l, t),
                     intSphere  (fpar00[1], ro, rd, l, t),
                     intCylinder(fpar00[2], ro, rd, l, t),
                     intCylinder(fpar00[3], ro, rd, l, t));
    return any(ss);
}


vec4 calc_shade(in vec3 po, in vec4 ob, in vec4 co, in vec3 rd, out vec4 re)
{
    float di,sp;
    vec2 uv;
    vec3 no;
    vec4 lz;

    lz.xyz = vec3(0.0f, 1.5f, -3.0f) - po;
    lz.w = length(lz.xyz);
    lz.xyz = lz.xyz / lz.w;

    no = calcnor(ob, co, po, uv);

    di = dot(no, lz.xyz);
    re.xyz = reflect(rd, no);
    sp = dot(re.xyz, lz.xyz);
    sp = max(sp, 0.0);
    sp = sp * sp;
    sp = sp * sp;

    if(calc_shadow(po, lz.xyz, lz.w))
        di = 0.0;

    di = max(di, 0.0);
    co = co * texture(tb_tex, uv) * (vec4(0.21f, 0.28f, 0.30f, 1.0f) + 0.5f * vec4(1.0f, 0.9f, 0.65f, 1.0f) * di) + sp;

    di = dot(no, -rd);
    re.w = di;
    di = 1.0 - di*di;
    di = di * di;

    return(co + 0.6 * vec4(di));
}

void main()
{

    vec2 uv = (vec2(gl_GlobalInvocationID.xy) + 0.5) / resolution;
    vec3 position = camera_ws;
    vec3 view = camera_matrix * vec3(focal_scale * (2.0f * uv - 1.0f), -1.0f);

    float tm;
    vec4 ob, co, co2, re, re2;
    vec3 no, po;
    vec3 ro = position;
    vec3 rd = normalize(view);

    tm = calc_inter(ro, rd, ob, co);

    po = ro + rd * tm;
    co = calc_shade(po, ob, co, rd, re);

    tm = calc_inter(po, re.xyz, ob, co2);
    po += re.xyz * tm;
    co2 = calc_shade(po, ob, co2, re.xyz, re2);

    vec4 FragmentColor = mix(co, co2, 0.5 - 0.5 * re.w) + vec4(fpar00[13].w);

    FragmentColor = co;
    
    imageStore(scene_image, ivec2(gl_GlobalInvocationID.xy), FragmentColor);
};
