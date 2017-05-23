#version 330 core

const float PI = 3.14159265;

uniform sampler2D texture0;
uniform sampler2D texture1;

uniform vec2 FocalLen;
uniform vec2 UVToViewA;
uniform vec2 UVToViewB;

uniform vec2 LinMAD;

uniform vec2 AORes;
uniform vec2 InvAORes;
uniform vec2 NoiseScale;

uniform float AOStrength;
uniform float R;
uniform float R2;
uniform float NegInvR2;
uniform float MaxRadiusPixels;

uniform float TanBias = tan(30.0 * PI / 180.0);

uniform int NumDirections;
uniform int NumSamples;

in vec2 TexCoord;

layout (location = 0) out float Occlusion;

vec3 UVToViewSpace(vec2 uv, float z)
{
    uv = UVToViewA * uv + UVToViewB;
    return vec3(uv * z, z);
}

vec3 GetViewPos(vec2 uv)
{
    float z = texture(texture0, uv).r;                              // camera-space z-value
    return UVToViewSpace(uv, z);
}

float TanToSin(float x)
    { return x * inversesqrt(x * x + 1.0f); }

float InvLength(vec2 V)
    { return inversesqrt(dot(V, V)); }

float Tangent(vec3 V)
    { return V.z * InvLength(V.xy); }

float BiasedTangent(vec3 V)
    { return V.z * InvLength(V.xy) + TanBias; }

float Tangent(vec3 P, vec3 S)
    { return -(P.z - S.z) * InvLength(S.xy - P.xy); }

float Length2(vec3 V)
    { return dot(V, V); }

vec3 MinDiff(vec3 P, vec3 Pr, vec3 Pl)
{
    vec3 V1 = Pr - P;
    vec3 V2 = P - Pl;
    return (Length2(V1) < Length2(V2)) ? V1 : V2;
}

vec2 SnapUVOffset(vec2 uv)
    { return round(uv * AORes) * InvAORes; }

float Falloff(float d2)
    { return d2 * NegInvR2 + 1.0f; }

float HorizonOcclusion(vec2 deltaUV, vec3 P, vec3 dPdu, vec3 dPdv, float randstep, float numSamples)
{
    float ao = 0;
    
    vec2 uv = TexCoord + SnapUVOffset(randstep*deltaUV);                            // Offset the first coord with some noise
    deltaUV = SnapUVOffset( deltaUV );
    
    vec3 T = deltaUV.x * dPdu + deltaUV.y * dPdv;                                   // Calculate the tangent vector
    
    float tanH = BiasedTangent(T);                                                  // Get the angle of the tangent vector from the viewspace axis
    float sinH = TanToSin(tanH);

    float tanS;
    float d2;
    vec3 S;
    
    for(float s = 1; s <= numSamples; ++s)                                          // Sample to find the maximum angle
    {
        uv += deltaUV;
        S = GetViewPos(uv);
        tanS = Tangent(P, S);
        d2 = Length2(S - P);
        
        if(d2 < R2 && tanS > tanH)                                                  // Is the sample within the radius and the angle greater?
        {
            float sinS = TanToSin(tanS);
            
            ao += Falloff(d2) * (sinS - sinH);                                      // Apply falloff based on the distance

            tanH = tanS;
            sinH = sinS;
        }
    }
    
    return ao;
}

vec2 RotateDirections(vec2 Dir, vec2 CosSin)
{
    return vec2(Dir.x * CosSin.x - Dir.y * CosSin.y,
                Dir.x * CosSin.y + Dir.y * CosSin.x);
}

void ComputeSteps(inout vec2 stepSizeUv, inout float numSteps, float rayRadiusPix, float rand)
{
    numSteps = min(NumSamples, rayRadiusPix);                                       // Avoid oversampling if numSteps is greater than the kernel radius in pixels
    
    float stepSizePix = rayRadiusPix / (numSteps + 1);                              // Divide by Ns + 1 so that the farthest samples are not fully attenuated
    
    float maxNumSteps = MaxRadiusPixels / stepSizePix;                              // Clamp numSteps if it is greater than the max kernel footprint
    if (maxNumSteps < numSteps)
    {
        numSteps = floor(maxNumSteps + rand);                                       // Use dithering to avoid AO discontinuities
        numSteps = max(numSteps, 1);
        stepSizePix = MaxRadiusPixels / numSteps;
    }

    stepSizeUv = stepSizePix * InvAORes;                                            // Step size in uv space
}

void main(void)
{
    float numDirections = NumDirections;

    vec3 P, Pr, Pl, Pt, Pb;
    P   = GetViewPos(TexCoord);
    
    Pr  = GetViewPos(TexCoord + vec2( InvAORes.x, 0));                              // Sample neighboring pixels
    Pl  = GetViewPos(TexCoord + vec2(-InvAORes.x, 0));
    Pt  = GetViewPos(TexCoord + vec2( 0, InvAORes.y));
    Pb  = GetViewPos(TexCoord + vec2( 0,-InvAORes.y));
    
    vec3 dPdu = MinDiff(P, Pr, Pl);                                                 // Calculate tangent basis vectors using the minimum difference
    vec3 dPdv = MinDiff(P, Pt, Pb) * (AORes.y * InvAORes.x);

    vec3 random = texture(texture1, TexCoord.xy * NoiseScale).rgb;                  // Get the random samples from the noise texture

    
    vec2 rayRadiusUV = 0.5 * R * FocalLen / -P.z;                                   // Calculate the projected size of the hemisphere
    float rayRadiusPix = rayRadiusUV.x * AORes.x;

    float ao = 1.0;
    
    if(rayRadiusPix > 1.0)                                                          // Make sure the radius of the evaluated hemisphere is more than a pixel
    {
        ao = 0.0;
        float numSteps;
        vec2 stepSizeUV;
        ComputeSteps(stepSizeUV, numSteps, rayRadiusPix, random.z);                 // Compute the number of steps
        float alpha = 2.0 * PI / numDirections;
        
        for(float d = 0; d < numDirections; ++d)                                    // Calculate the horizon occlusion of each direction
        {
            float theta = alpha * d;
            vec2 dir = RotateDirections(vec2(cos(theta), sin(theta)), random.xy);   // Apply noise to the direction
            vec2 deltaUV = dir * stepSizeUV;
            ao += HorizonOcclusion(deltaUV, P, dPdu, dPdv, random.z, numSteps);     // Sample the pixels along the direction
        }
        ao = 1.0 - ao / numDirections * AOStrength;                                 // Average the results and produce the final AO
    }

    Occlusion = ao;
}