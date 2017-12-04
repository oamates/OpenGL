// File that allows to manage primitives in parallel with glsl

#include <glm/gtx/string_cast.hpp>

#include "primitive.hpp"

 // Triangle
std::ostream& operator << (std::ostream& stream, const Triangle& parTriangle)
{
	stream << "P0 = " << glm::to_string(parTriangle.p0) << std::endl;
	stream << "P1 = " << glm::to_string(parTriangle.p1) << std::endl;
	stream << "P2 = " << glm::to_string(parTriangle.p2) << std::endl;
	stream << "N  = " << glm::to_string(parTriangle.normale) << std::endl;
	return stream;
}

// Materiau 
std::ostream& operator << (std::ostream& stream, const Materiau& parMat)
{
	stream << "color " << glm::to_string(parMat.color) << std::endl;
	stream << "refractance " << parMat.coeffReflexion << std::endl;
	stream << "reflectance " << parMat.coeffRefraction << std::endl;
	return stream;
}

// Primitive 
std::ostream& operator << (std::ostream& stream, const Primitive& parTriangle)
{
	stream << "type " << parTriangle.type << std::endl;
	stream << "index " << parTriangle.index << std::endl;
	stream << "materiau " << parTriangle.materiau << std::endl;
	return stream;
}


// Lumière
std::ostream& operator << (std::ostream& stream, const Light& parLum)
{
	stream << "color " << glm::to_string(parLum.color) << std::endl;
	stream << "intensity " << parLum.intensity << std::endl;
	stream << "position " << glm::to_string(parLum.position) << std::endl;
	return stream;
}
