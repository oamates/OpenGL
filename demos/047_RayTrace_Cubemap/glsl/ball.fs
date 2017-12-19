        ObjectDesc("Ball fragment shader"),
        StrCRef("#version 150\n"
        "uniform vec3 Color1, Color2;"
        "uniform sampler2DArray NumberTex;"
        "uniform samplerCube ReflectTex;"
        "uniform int BallIdx;"
        "in vec3 geomNormal;"
        "in vec3 geomTangent;"
        "in vec3 geomBitangent;"
        "in vec3 geomLightDir;"
        "in vec3 geomViewDir;"
        "in vec2 geomTexCoord;"
        "in vec2 geomSTCoord;"
        "out vec4 fragColor;"

        "void main(void)"
        "{"
        "   vec3 TexCoord = vec3(geomTexCoord, float(BallIdx));"
        "   vec4 Sample = texture(NumberTex, TexCoord);"
        "   vec3 LightColor = vec3(1.0, 1.0, 1.0);"

        "   vec3 Normal = normalize(geomNormal);"
        "   vec3 LightDir = normalize(geomLightDir);"
        "   vec3 ViewDir = normalize(geomViewDir);"
        "   vec3 LightRefl = reflect(-LightDir, Normal);"
        "   vec3 ViewRefl = reflect(-ViewDir, Normal);"

        "   vec3 ReflSample = texture(ReflectTex, ViewRefl).rgb;"

        "   float Specular = pow(max(dot("
        "       LightRefl,"
        "       normalize(geomViewDir)"
        "   )+0.1, 0.0), 64.0)*(0.5 - Sample.a*(1.0-Sample.r)*0.4);"

        "   float Diffuse = max(dot(Normal, LightDir)+0.1, 0.0);"

        "   const float Reflectivity = 0.2;"
        "   const float Ambient = 0.2;"

        "   float ColorSwitch = ("
        "       geomSTCoord.t < 0.25 ||"
        "       geomSTCoord.t > 0.75"
        "   )? 0.0 : 1.0;"
        "   vec3 Color = mix("
        "       mix(Color1, Color2, ColorSwitch),"
        "       vec3(1.0, 1.0, 0.9)*Sample.r,"
        "       Sample.a"
        "   );"

        "   fragColor = vec4("
        "       ReflSample * Reflectivity +"
        "       Color * Ambient +"
        "       LightColor * Color * Diffuse +"
        "       LightColor * Specular, "
        "       1.0"
        "   );"
        "}")
    )
    { }
