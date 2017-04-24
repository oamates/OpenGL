#version 430 core

const uint N = 512;
const float GLUS_PI	= 3.1415926535897932384626433832795;

uniform int u_processColumn;
uniform int u_steps;

layout (binding = 0, rg32f) uniform image2D u_imageIn; 
layout (binding = 1, rg32f) uniform image2D u_imageOut;

layout (binding = 2, r32ui) uniform uimage1D u_imageIndices;

shared vec2 sharedStore[N];

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

vec2 mulc(vec2 a, vec2 b)
{
	vec2 result;	
	result.x = a.x * b.x - a.y * b.y;
	result.y = a.x * b.y + b.x * a.y;	
	return result;
}

vec2 rootOfUnityc(int n, int k)
{
	vec2 result;	
	result.x = cos(2.0 * GLUS_PI * float(k) / float(n));
	result.y = sin(2.0 * GLUS_PI * float(k) / float(n));
	return result;
}

void main(void)
{
	ivec2 leftStorePos;
	ivec2 rightStorePos;

	ivec2 leftLoadPos;
	ivec2 rightLoadPos;

	uint xIndex = gl_GlobalInvocationID.x;
	uint yIndex = gl_GlobalInvocationID.y;

	uint leftStoreIndex = 2 * xIndex;
	uint rightStoreIndex = leftStoreIndex + 1;
	uint leftLoadIndex = imageLoad(u_imageIndices, leftStoreIndex).r;								// Load the swizzled indices.
	uint rightLoadIndex = imageLoad(u_imageIndices, rightStoreIndex).r;
	
	if (u_processColumn == 0)																		// Loading and storing position depends on processing per row or column.
	{
		leftLoadPos = ivec2(leftLoadIndex, yIndex);
		rightLoadPos = ivec2(rightLoadIndex, yIndex);
		leftStorePos = ivec2(leftStoreIndex, yIndex);
		rightStorePos = ivec2(rightStoreIndex, yIndex);
	}
	else
	{
		leftLoadPos = ivec2(yIndex, leftLoadIndex);
		rightLoadPos = ivec2(yIndex, rightLoadIndex);
		leftStorePos = ivec2(yIndex, leftStoreIndex);
		rightStorePos = ivec2(yIndex, rightStoreIndex);
	}
	
	vec2 leftValue = imageLoad(u_imageIn, leftLoadPos).xy;											// Copy and swizzle values for butterfly algortihm into the shared memory.
	vec2 rightValue = imageLoad(u_imageIn, rightLoadPos).xy;
	sharedStore[leftStoreIndex] = leftValue;
	sharedStore[rightStoreIndex] = rightValue;
	
	memoryBarrierShared();																			// Make sure that all values are stored and visible after the barrier. 
	barrier();
	
	uint numberSections = N / 2;
	uint numberButterfliesInSection = 1;

	uint currentSection = xIndex;
	uint currentButterfly = 0;
	
	for (int currentStep = 0; currentStep < u_steps; currentStep++)									// Performing needed FFT steps per either row or column.
	{	
		int leftIndex = currentButterfly + currentSection * numberButterfliesInSection * 2;
		int rightIndex = currentButterfly + numberButterfliesInSection + currentSection * numberButterfliesInSection * 2;
	
		leftValue = sharedStore[leftIndex];
		rightValue = sharedStore[rightIndex];
	
		// "Butterfly" math.
		
		vec2 currentW = rootOfUnityc(numberButterfliesInSection * 2, currentButterfly);				// Note: To improve performance, put root of unity calculation in look up texture or buffer.
	
		vec2 multiply;
		vec2 addition;
		vec2 subtraction;

		multiply = mulc(currentW, rightValue);	
		
		addition = leftValue + multiply;
		subtraction = leftValue - multiply; 

		sharedStore[leftIndex] = addition;
		sharedStore[rightIndex] = subtraction;		

		// Make sure, that values are written.		
		memoryBarrierShared();

		// Change parameters for butterfly and section index calculation.		
		numberButterfliesInSection *= 2;
		numberSections /= 2;

		currentSection /= 2;
		currentButterfly = xIndex % numberButterfliesInSection;

		// Make sure, that all shaders are at the same stage, as now indices are changed.
		barrier();
	}
	
	
	if (u_processColumn == 1)																		// Process twiddle factor for second pass FFT. 
	{
		if ((leftStorePos.x + leftStorePos.y) % 2 == 0)
		{
			sharedStore[leftStoreIndex] *= -1.0;
		}
		if ((rightStorePos.x + rightStorePos.y) % 2 == 0)
		{
			sharedStore[rightStoreIndex] *= -1.0;			
		}
		memoryBarrierShared();																		// Make sure, that values are written.
	}
	
	// Store from shared to global memory.
	imageStore(u_imageOut, leftStorePos, vec4(sharedStore[leftStoreIndex], 0.0, 0.0));
	imageStore(u_imageOut, rightStorePos, vec4(sharedStore[rightStoreIndex], 0.0, 0.0));
}
