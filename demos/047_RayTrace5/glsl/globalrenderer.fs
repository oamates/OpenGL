#version 450 core

#define M_PI 3.14159
#define M_SQRT3 1.732
#define gammacorr(x) pow(x, 1/2.6)

struct Material
{
	vec3 color_diffuse;
	vec3 color_specular;
	float reflectance; // [0, 1]
	float specular_reflectance; // [0, 1], part of reflectance
	float specular_cone_angle; // [0, 1]
	float radiosity;
};

struct LightSource
{
	float intensity;
	float radius;
	vec3 color;
	vec3 position;
};

in vec3 normal_viewspace;
in vec3 vertexPosition_viewspace;
in vec3 vertexPosition_worldspace;
in vec3 normal_worldspace;
in vec3 eyePosition_worldspace;

uniform float time;
uniform int textureSize;
uniform float sceneScale;
uniform sampler3D texUnit3D;
uniform Material material;
uniform LightSource light;


out vec4 color;

float random(float xx){
	float x0 = floor(xx);
	float x1 = x0+1;
	float v0 = sin( x0*14686 ) * 31718.927 + x0;
	float v1 = sin( x1*14686 ) * 31718.927 + x1;
	v0 = v0 - int(v0);
	v1 = v1 - int(v1);
	float fracxx = xx - int(xx);
	return (v0*(1-fracxx)+v1*(fracxx))*2-1*sin(xx);
}

// Trace cone from position vertexPosition_worldspace and accumulate color
vec3 coneTrace(vec3 rayDirection, float coneAngle, float multiSample, int steps)
{
	//coneAngle = M_PI / 30;
	vec4 res = vec4(0,0,0,0);
	float tanTheta2 = tan(coneAngle / 2);
	float tanAlpha2 = tan(coneAngle / (2*multiSample));
	float sampleFactor = (1 + tanAlpha2) / (1 - tanAlpha2);
	float voxelSize = float(1) / textureSize * 2;

	vec3 rayOrigin = vertexPosition_worldspace + voxelSize * sceneScale * M_SQRT3 * normalize(normal_worldspace);
	float sampleStep = voxelSize / 2;
	float t = 0;//0.05 * sampleFactor * random(123314 *( 1 + time) *(vertexPosition_viewspace.x + vertexPosition_viewspace.y + vertexPosition_viewspace.z));
	for (int i=0; i<steps; i++)
	{
		// Increment sampleStep
		sampleStep = sampleStep * sampleFactor;
		/*if (sampleStep > 0.05)
		{
			sampleStep = 0.05;
		}*/
		t += sampleStep;

		float d = (tanTheta2 * t * 2); // Sphere diameter
		float mipLevel = log2(d / voxelSize);
		/*
		if (mipLevel > log2(textureSize) - 1)
			mipLevel = log2(textureSize) - 1;
		*/
		/*
		if (sampleStep * multiSample > textureSize / 2)
		{
			break;
		}*/

		vec3 samplePoint = (rayOrigin + rayDirection * t );
		if (samplePoint.x < -sceneScale || samplePoint.x > sceneScale || 
			samplePoint.y < -sceneScale || samplePoint.y > sceneScale ||
			samplePoint.z < -sceneScale || samplePoint.z > sceneScale)
		{
			break;
		}
		vec4 texSample = textureLod(texUnit3D, (samplePoint / sceneScale + vec3(1,1,1)) / 2, mipLevel) * sampleStep * 100;
		
		/*
		if (texSample.a > 0)
		{
			//texSample.rgb /= texSample.a;
			// Alpha compositing
			//res.rgb = res.rgb + (1 - res.a) * texSample.rgb;
	        //res.a   = res.a   + (1 - res.a) * texSample.a;
	        res.a = res.a + texSample.a * (1 - res.a);
	        res.rgb = (res.rgb * res.a + texSample.rgb * texSample.a * (1 - res.a)) / res.a;
		}
		if (res.a > 0.9)
			break;
			*/
		
		
		if (texSample.a > 0)
		{
			texSample.rgb /= texSample.a;
			// Alpha compositing
			res.rgb = res.rgb + (1 - res.a) * texSample.a * texSample.rgb;
	        res.a   = res.a   + (1 - res.a) * texSample.a;
		}
		if (res.a > 0.9)
		{
			break;
		}

			
		/*
		res += texSample;
		if (res.r > 0 || res.g > 0 || res.b > 0)
		{
			break;
		}*/
	}
	return res.rgb;
}

vec3 calculateLocalDiffuse()
{
	vec3 n = normalize(normal_worldspace);
	vec3 light_diff = vertexPosition_worldspace - light.position;
	float light_dist = length(light_diff);
	vec3 l = normalize(light_diff);


	float cosTheta = dot(n,-l);
	vec3 diffuse =
		light.color * light.intensity *
		max(cosTheta, 0) *
		1 / pow(light_dist, 2);

	return diffuse;
}

vec3 calculateGlobalDiffuse(vec3 n_worldspace)
{
	vec3 n = n_worldspace;
   	vec3 rayDirection = n;
   	vec3 res;
   	float coneAngle = M_PI / 3;

   	// Pick a random vector helper
   	vec3 helper = vec3(0.12,0.32,0.82);// normalize(vec3(random(int(vertexPosition_worldspace.x * 5226)),random(int(vertexPosition_worldspace.y * 5226)),random(int(vertexPosition_worldspace.z * 5226))));
   	if (abs(dot(n,helper)) == 1)
   		// Pick a new helper
   		helper = vec3(0,1,0);

   	// Find a tangent and a bitangent
   	vec3 t = normalize(helper - dot(n,helper) * n);
   	vec3 bt = cross(t, n);
   	
   	float multiSample = 4;
   	/*
   	// First trace a cone in the normal direction
   	res += coneTrace(rayDirection, coneAngle, multiSample, 200);
   	float inclination = M_PI / 4;

   	for (int i = 0; i < 5; ++i)
   	{
   		rayDirection = n * cos(inclination) + sin(inclination) * (cos( i * 2 * M_PI / 5) * t + sin(i * 2 * M_PI / 5) * bt);
   		res += coneTrace(normalize(rayDirection), coneAngle, multiSample, 200);
   	}

   	return res / 6;
   	*/
   	
   	// First trace a cone in the normal direction
   	res += coneTrace(rayDirection, coneAngle, multiSample, 200);

   	// Rotate the ray direction 30 degrees around the tangent to achieve the next
   	// ray direction
   	rayDirection = 0.7071 * n + 0.7071 * t;
   	res += coneTrace(rayDirection, coneAngle, multiSample, 200);

   	// Next sample direction
   	rayDirection = 0.7071 * n + 0.7071 * (0.309 * t + 0.951 * bt);
   	res += coneTrace(rayDirection, coneAngle, multiSample, 200);
  	// Next sample direction
   	rayDirection = 0.7071 * n + 0.7071 * (-0.809 * t + 0.588 * bt);
   	res += coneTrace(rayDirection, coneAngle, multiSample, 200);
	// Next sample direction
   	rayDirection = 0.7071 * n - 0.7071 * (-0.809 * t - 0.588 * bt);
   	res += coneTrace(rayDirection, coneAngle, multiSample, 200);
   	// Next sample direction
   	rayDirection = 0.7071 * n - 0.7071 * (0.309 * t - 0.951 * bt);
   	res += coneTrace(rayDirection, coneAngle, multiSample, 200);

   	return res / 6;
}

vec3 calculateLocalSpecular()
{
	vec3 n = normalize(normal_worldspace);
	vec3 v = normalize( vertexPosition_worldspace - eyePosition_worldspace );
	vec3 r = reflect(v, n);

	vec3 light_diff = vertexPosition_worldspace - light.position;
	float light_dist = length(light_diff);
	vec3 l = normalize(light_diff);

	float cos_alpha = dot(r,-l);


	vec3 spcular =
	light.color * light.intensity *
	pow(max(cos_alpha, 0), (1 / (material.specular_cone_angle + 0.1)) * 20);// *
	/*1 / pow(light_dist, 2);*/

	return spcular;
}

vec3 calculateGlobalSpecular()
{
	vec3 v = normalize( vertexPosition_worldspace - eyePosition_worldspace );
	vec3 r = reflect(v, normalize(normal_worldspace));
	float cone_angle = material.specular_cone_angle;//atan((1 - material.specular_cone_angle) * M_PI / 2);
	return coneTrace(r, cone_angle, 4, 200);
}

vec3 calculateGlobalDirectDiffuse()
{
	vec3 n = normalize(normal_worldspace);
	vec3 light_diff = light.position - vertexPosition_worldspace;
	float light_dist = length(light_diff);
	vec3 l = normalize(light_diff);

	float cone_angle = atan(light.radius / light_dist) * 2;
	float cosTheta = dot(n,l);

	vec3 diffuse =
		coneTrace(l, cone_angle, 8, 200) *
		max(cosTheta, 0) *
		1 / pow(light_dist, 2);

	return diffuse / 3;
}

void main(){
	color = vec4(0,0,0,1);
	// Add diffuse
	if (material.reflectance != 0 && (1 - material.specular_reflectance) != 0)
	{
		// Add diffuse
		//color.rgb = calculateLocalDiffuse() * material.color_diffuse * material.reflectance * (1 - material.specular_reflectance);
		color.rgb += calculateGlobalDirectDiffuse() * material.color_diffuse * material.reflectance * (1 - material.specular_reflectance);
		color.rgb += calculateGlobalDiffuse(normalize(normal_worldspace)) * material.color_diffuse * material.reflectance * (1 - material.specular_reflectance);
	}
	// Add specular
	if (material.reflectance != 0 && material.specular_reflectance != 0)
	{
		color.rgb += calculateGlobalSpecular() * material.color_specular * material.reflectance * material.specular_reflectance;
		color.rgb += calculateLocalSpecular() * material.color_specular * material.reflectance * material.specular_reflectance;
	}
	color.rgb += material.radiosity * material.color_diffuse;

	color.rgb = vec3(gammacorr(color.r), gammacorr(color.g), gammacorr(color.b));
}