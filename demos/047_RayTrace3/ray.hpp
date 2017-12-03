#include "glm/glm.hpp"

struct ray
{
	glm::vec3 origin;
	float padding0;	
	glm::vec3 dir;
	float padding1;
	glm::vec3 color;
	float t;
	int primitiveID;
	float padding2;
	float padding3;
	float padding4;
	// the surface normal from the last primitive this ray 
	// intersected with
	glm::vec3 n;
	float padding5;	
};
