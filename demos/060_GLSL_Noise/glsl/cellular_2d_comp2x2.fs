#version 330

in vec2 v_texCoord2D;

vec4 permute(vec4 x) 
    { return mod((34.0f * x + 1.0f) * x, 289.0f); }

//==============================================================================================================================================================
// Simplified cellular noise : 2x2 search window
//==============================================================================================================================================================

vec2 cellular2x2 (vec2 P)
{
    const float K = 1.0f / 7.0f;
    const float K2 = 0.5f / 7.0f;
    const float jitter = 0.8f;                                      // jitter 1.0 makes F1 wrong more often
    vec2 Pi = mod (floor(P), 289.0f);
    vec2 Pf = fract (P);
    vec4 Pfx = Pf.x + vec4(-0.5f, -1.5f, -0.5f, -1.5f);
    vec4 Pfy = Pf.y + vec4(-0.5f, -0.5f, -1.5f, -1.5f);
    vec4 p = permute(Pi.x + vec4(0.0f, 1.0f, 0.0f, 1.0f));
    p = permute (p + Pi.y + vec4 (0.0f, 0.0f, 1.0f, 1.0f));
    vec4 ox = mod (p, 7.0f) * K + K2 ;
    vec4 oy = mod (floor(p * K), 7.0f) * K + K2;
    vec4 dx = Pfx + jitter * ox;
    vec4 dy = Pfy + jitter * oy;
    vec4 d = dx * dx + dy * dy;                                     // distances squared. cheat and pick only F1 for the return value
    d.xy = min (d.xy, d.zw);
    d.x = min (d.x, d.y);
    return d.xx;                                                    // F1 duplicated , F2 not computed
}




out vec4 FragmentColor;

void main()
{
    vec2 F = cellular2x2(v_texCoord2D);
    FragmentColor = vec4(F.x, F.y, 0.0, 1.0);
}
