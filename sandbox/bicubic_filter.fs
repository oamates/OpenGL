vec4 cubic(float v)
{
    vec4 n = vec4(1.0, 2.0, 3.0, 4.0) - v;
    vec4 s = n * n * n;
    float x = s.x;
    float y = s.y - 4.0 * s.x;
    float z = s.z - 4.0 * s.y + 6.0 * s.x;
    float w = 6.0 - x - y - z;
    return vec4(x, y, z, w) * (1.0/6.0);
}

vec4 textureBicubic(sampler2D sampler, vec2 texCoords)
{

   vec2 texSize = textureSize(tex, 0);
   vec2 invTexSize = 1.0 / texSize;
   
   texCoords = texCoords * texSize - 0.5;

   
    vec2 fxy = fract(texCoords);
    texCoords -= fxy;

    vec4 xcubic = cubic(fxy.x);
    vec4 ycubic = cubic(fxy.y);

    vec4 c = texCoords.xxyy + vec2(-0.5, +1.5).xyxy;
    
    vec4 s = vec4(xcubic.xz + xcubic.yw, ycubic.xz + ycubic.yw);
    vec4 offset = c + vec4(xcubic.yw, ycubic.yw) / s;
    
    offset *= invTexSize.xxyy;
    
    vec4 sample0 = texture(sampler, offset.xz);
    vec4 sample1 = texture(sampler, offset.yz);
    vec4 sample2 = texture(sampler, offset.xw);
    vec4 sample3 = texture(sampler, offset.yw);

    float sx = s.x / (s.x + s.y);
    float sy = s.z / (s.z + s.w);

    return mix(
       mix(sample3, sample2, sx), mix(sample1, sample0, sx)
    , sy);
}


//===============================================================================================================================================================================
//===============================================================================================================================================================================
//===============================================================================================================================================================================
//===============================================================================================================================================================================
//===============================================================================================================================================================================

// this work is licenced under a CC0 1.0 Universal License
// https://creativecommons.org/publicdomain/zero/1.0/

#define LEVELS (floor(log2(min(iResolution.x, iResolution.y)))-1.0)

// sample
vec4 myTexture( sampler2D channel, ivec2 uv, ivec2 level ) {
    // clamp level
    level = clamp(level, ivec2(0), ivec2(LEVELS));
    // figure out the resolution
	ivec2 res = ivec2(1) << (ivec2(LEVELS)-level);
    // texture wrapping
    uv = uv % res;
    return texelFetch(channel, res+uv-1, 0);
}

// bilinear interpolation
vec4 myTexture( sampler2D channel, vec2 uv, ivec2 level ) {
	// find the texels
    vec2 res = exp2(LEVELS-vec2(level));
    uv = uv*res - 0.5;
    ivec2 a = ivec2(uv - step(uv, vec2(0)));
    vec2 x = uv - vec2(a);
    // lerp
    const ivec2 d = ivec2(1, 0);
    vec4 left  = mix(myTexture(channel, a+d.yy, level), myTexture(channel, a+d.yx, level), x.y);
    vec4 right = mix(myTexture(channel, a+d.xy, level), myTexture(channel, a+d.xx, level), x.y);
    return mix(left, right, x.x);
}

// trilinear interpolation
vec4 myTexture( sampler2D channel, vec2 uv, vec2 level ) {
    // find the correct level
    ivec2 a = ivec2(level);
    vec2 x = level - vec2(a);
    // lerp
    const ivec2 d = ivec2(1, 0);
    vec4 left  = mix(myTexture(channel, uv, a+d.yy), myTexture(channel, uv, a+d.yx), x.y);
    vec4 right = mix(myTexture(channel, uv, a+d.xy), myTexture(channel, uv, a+d.xx), x.y);
    return mix(left, right, x.x);
}

// filtered texture lookup, analogous to textureGrad(channel, uv, dx, dy)
vec4 myTextureGrad( sampler2D channel, vec2 uv, vec2 dx, vec2 dy ) {
    vec2 box = max(abs(dx), abs(dy)) * exp2(LEVELS) * 2.0;
    box = clamp(log2(box), vec2(0.0), vec2(16.0));
    return myTexture(channel, uv, max(vec2(0.0), box));
}

// analogous to texture(channel, uv)
vec4 myTexture( sampler2D channel, vec2 uv ) {
    return myTextureGrad(channel, uv, dFdx(uv), dFdy(uv));
}

/////////////////////////////
// TEXTURE FILTERING ABOVE //
/////////////////////////////

#define PI 3.14159265359
#define rot(a) mat2(cos(a + PI*0.25*vec4(0,6,2,0)))

// strings
const ivec2[] text0 = ivec2[]( 
	ivec2(7,9),  ivec2(12,9), ivec2(7,11), ivec2(5,9),  ivec2(14,9), 
	ivec2(5,9),  ivec2(2,8),  ivec2(1,9),  ivec2(4,8),  ivec2(5,9),  
	ivec2(13,11),ivec2(9,9),  ivec2(0,8),  ivec2(13,9), ivec2(1,9),  
	ivec2(0,8),  ivec2(8,13), ivec2(9,13));
const ivec2[] text1 = ivec2[]( 
	ivec2(0,13), ivec2(0,13), ivec2(4,8),  ivec2(5,9),  ivec2(8,8),  
	ivec2(4,8),  ivec2(5,8),  ivec2(2,8),  ivec2(5,9),  ivec2(7,11), 
	ivec2(2,8),  ivec2(1,9),  ivec2(4,9),  ivec2(8,13), ivec2(9,13), 
	ivec2(0,13), ivec2(0,13), ivec2(0,13));
const ivec2[] text2 = ivec2[]( 
	ivec2(0,13), ivec2(0,13), ivec2(0,13), ivec2(0,13), ivec2(4,8),  
	ivec2(5,9),  ivec2(8,8),  ivec2(4,8),  ivec2(5,8),  ivec2(2,8),  
	ivec2(5,9),  ivec2(8,13), ivec2(9,13), ivec2(0,13), ivec2(0,13), 
	ivec2(0,13), ivec2(0,13), ivec2(0,13));

vec4 text( vec2 uv, int id ) {
    ivec2 string[] = text0;
    if (id == 1) string = text1;
    if (id == 2) string = text2;
    uv *= 20.0;
    float alias = length(fwidth(uv))*0.5;
    uv += vec2(float(string.length())*0.5, 0.5);
    float letter = 0.0;
    ivec2 iuv = ivec2(floor(uv));
    vec2 fuv = fract(uv);
    vec3 col = vec3(0);
    if ( iuv.x>=0 && iuv.x<=string.length()-1 && iuv.y==0 ) {
        float te = texture( iChannel1, ( vec2(string[iuv.x]*64) + fuv*64.0 )/1024.0 ).w;
        letter = smoothstep( alias, -alias, te-0.551 );
        col += smoothstep( alias, -alias, te-0.5 );
    }
    return vec4(col, letter);
}


// trace to a plane
float plane( vec4 pl, vec3 ro, vec3 rd ) {
    return -(dot(pl.xyz,ro)+pl.w)/dot(pl.xyz,rd);
}

void mainImage( out vec4 fragColor, vec2 fragCoord ) {
    
    vec2 uv = (fragCoord.xy - iResolution.xy * 0.5) / iResolution.x;
    
    if (iTime < 5.0) {

        // display the mip map construction
        fragColor = texelFetch(iChannel0, ivec2(fragCoord), 0);
        vec4 string = text(uv, 0);
        fragColor.rgb = mix(fragColor.rgb, string.rgb, string.a);
        
    } else if (iTime < 15.0) {
        
        // display textureGrad behavior
        vec2 box = vec2(cos(iTime*vec2(0.752, 0.834)))*0.03+0.03;
        if (iMouse.z > 0.0) {
            box = abs(iMouse.xy - iResolution.xy * 0.5)/iResolution.x;
        }
        
        vec2 inBox = abs(uv)-box;
        float dBox = max(inBox.x, inBox.y);
        float border = smoothstep(0.0, 0.005, abs(dBox));
        
        //box *= step(dBox, 0.0);
        
        fragColor = myTextureGrad(iChannel0, uv, vec2(box.x, 0), vec2(0, box.y));
        fragColor.rgb = mix(vec3(0, 1, 0), fragColor.rgb, border);
        
        vec4 string = text(uv, 1);
        fragColor.rgb = mix(fragColor.rgb, string.rgb, string.a);
        
    } else {
    	
        // display a raytraced plane
        vec3 from = vec3(0, 2.0+iMouse.y*0.05, iTime*10.0);
        vec3 dir = normalize(vec3(uv, 0.4));
        dir.xz *= rot(iTime*0.1);
        dir.y = dir.y;
		
        float dist = plane( vec4(0, 1, 0, 0), from, dir );
		
        vec3 p = from+dir*dist;
        fragColor = myTexture(iChannel0, p.xz*0.1);
        
        if (dist < 0.0) fragColor = vec4(0,0,0,1);
        
        vec4 string = text(uv, 2);
        fragColor.rgb = mix(fragColor.rgb, string.rgb, string.a*smoothstep(18.0, 17.0, iTime));
        
    }
    
    // gamma correction
    fragColor.rgb = pow(fragColor.rgb, vec3(1.0/2.2));

}