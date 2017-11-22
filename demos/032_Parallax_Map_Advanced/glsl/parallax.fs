#version 330 core

in vec3 vertWorldPos;

const int FLAT = 0;
const int NORMAL = 1;
const int PARALLAX = 2;

uniform sampler2D colorTexture;
uniform sampler2D normalsTexture;
uniform sampler2D dispTexture;
uniform sampler2D specularTexture;

uniform float amplitude = 0.0f;

uniform int mode = PARALLAX;
uniform int nbLayers = 20;
uniform int interpolation = 1;
uniform int selfShadow = 1;
uniform int crop = 0;
uniform int specularMapping = 1;

uniform vec3 eyeWorldPos;
uniform vec3 lightWorldPos;

out vec4 FragmentColor;


vec3 extractNormal (vec4 color)
{
    vec3 normal = color.rgb - 0.5;
    normal.xy *= amplitude / 0.05;
    return normalize(normal);
}

float extractDepth (vec4 color)
{
    return color.r;
}

float computeAmbient ()
{
    return 1.0;
}

float computeDiffuse (const vec3 normalizedNormal, const vec3 normalizedToLight)
{
    return max(0.0, dot(normalizedNormal, normalizedToLight));
}

float computeSpecular (const vec3 normalizedNormal, const vec3 normalizedToLight, const vec3 normalizedToEye)
{
    vec3 H = normalize(normalizedToEye + normalizedToLight); //for blinnphong
    return pow(max(0.0, dot(normalizedNormal,H)), 2); //blinnphong;
}

void main( void )
{
    vec2 coordsOnTex = vertWorldPos.xy;
    vec3 fragWorldPos = vertWorldPos;
    vec3 normalizedNormal = vec3(0.0f, 0.0f, 1.0f);
    float selfShadowFactor = 1.0;
    
    if (crop != 0)
    {
        if (coordsOnTex.x < 0 || coordsOnTex.y < 0 || coordsOnTex.x > 1 || coordsOnTex.y > 1)
            discard;
    }
    
    if (mode == PARALLAX)
    {
        float layerDepth = 1.0 / float(nbLayers);
        
        /* First parallax computation */
        //eye -> frag, normalized so that .z = amplitude * layerDepth
        vec3 fromEye = fragWorldPos - eyeWorldPos;
        fromEye *= amplitude * layerDepth / (-fromEye.z);
        
        float currLayerDepth = 0;
        vec2 prevTexCoords = coordsOnTex, nextTexCoords = prevTexCoords;
        float prevDepth = extractDepth(texture(dispTexture, coordsOnTex)), nextDepth = prevDepth;
        while (currLayerDepth < nextDepth) {
            prevTexCoords = nextTexCoords;
            nextTexCoords = prevTexCoords + fromEye.xy;
            
            prevDepth = nextDepth;
            nextDepth = extractDepth(texture(dispTexture, nextTexCoords));
            
            currLayerDepth += layerDepth;
        }
        
        float r1 = currLayerDepth - nextDepth;
        float r2 = prevDepth - (currLayerDepth - layerDepth);
        float r = r1 / (r1 + r2) * float(interpolation);
        
        coordsOnTex = mix(nextTexCoords, prevTexCoords, r);
        
        /* Then self-shadowing computation */
        // frag -> light, so that .z = amplitude * layerDepth
        vec3 toLight = lightWorldPos;
        toLight *= amplitude * layerDepth / toLight.z;
        
        vec2 tmpCoords = coordsOnTex;
        float currMapDepth = extractDepth(texture(dispTexture, tmpCoords));
        currLayerDepth = currMapDepth;
        while (currLayerDepth > 0)
        {
            tmpCoords += toLight.xy;
            currLayerDepth -= layerDepth;
            currMapDepth = extractDepth(texture(dispTexture, tmpCoords));
            
            if (currMapDepth < currLayerDepth)
                selfShadowFactor -= (currLayerDepth-currMapDepth) / layerDepth;
        }
        selfShadowFactor = max(0, selfShadowFactor);
    }
    
    if (mode != FLAT)
        normalizedNormal = extractNormal(texture(normalsTexture, coordsOnTex));
    
    vec3 normalizedToEye = normalize(eyeWorldPos - fragWorldPos);
    vec3 normalizedToLight = normalize(lightWorldPos);
    
    if (selfShadow == 0)
        selfShadowFactor = 1.0;
        
    float specularFactor = texture(specularTexture, coordsOnTex).r;
    if (specularMapping == 0)
        specularFactor = 1.0;
    
    vec4 color = texture(colorTexture, coordsOnTex);
    vec4 fragColor = vec4(0);
    
    fragColor += color * computeAmbient() * 0.5;
    fragColor += color * selfShadowFactor * computeDiffuse(normalizedNormal, normalizedToLight) * 0.5;
    fragColor += color * selfShadowFactor * specularFactor * computeSpecular(normalizedNormal, normalizedToLight, normalizedToEye) * 1;

    FragmentColor = fragColor;
}
