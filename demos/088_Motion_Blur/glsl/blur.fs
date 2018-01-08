#version 330 core

uniform sampler2DRect current_frame;
uniform sampler2DRect previous_frames;
uniform float splitter;

in vec2 uv;
out vec4 FragmentColor;

vec3 sharp()
{
   return texture(current_frame, uv).rgb;
}

vec3 blurred()
{
    vec3 prev = vec3(0, 0, 0);
    const vec2 otc[9] = vec2[9]
    (
        vec2(-1.0f,-1.0f),
        vec2( 0.0f,-1.0f),
        vec2( 1.0f,-1.0f),
        vec2(-1.0f, 0.0f),
        vec2( 0.0f, 0.0f),
        vec2( 1.0f, 0.0f),
        vec2(-1.0f, 1.0f),
        vec2( 0.0f, 1.0f),
        vec2( 1.0f, 1.0f)
    );

    const float is = 1.0 / 9.0;
    for(int s = 0; s != 9; ++s)
    {
        vec2 tc = uv + otc[s] * 2;
        prev += texture(previous_frames, tc).rgb * is;
    }
    vec3 curr = texture(current_frame, uv).rgb;
    float a = curr.x + curr.y + curr.z;
    return mix(curr, prev, 0.95 - 0.4 * a);
}

void main()
{
    if (gl_FragCoord.x < splitter - 1.0f)
        FragmentColor = vec4(sharp(), 1.0f);
    else if (gl_FragCoord.x > splitter + 1.0f)
        FragmentColor = vec4(blurred(), 1.0f);
    else FragmentColor = vec4(1.0f);
}
