#include <cmath>

#include <glm/gtx/transform.hpp> 

#include "camera.hpp"
#include "shadermanager.hpp"
#include "defines.hpp"

Camera::Camera()
: FTransformation(1.0)
, FLens(50.0)
, FAngleView(45.0)
{
	// Calul de la hayeur de l'ecran
	FHauteurEcran =  tan(FAngleView/2)*FLens;
	FPasX = SCREEN_X;
	FPasY = SCREEN_Y;
}

Camera::~Camera()
{
	
}

// Rotation autour de Y
void Camera::Yaw(double parAngle)
{
	FTransformation = FTransformation * glm::rotate(parAngle, glm::dvec3(0.0, 1.0, 0.0));
}
// Translation d'un vecteur
void Camera::Translate(const glm::dvec3& parDir)
{
	FTransformation = FTransformation * glm::translate(parDir);
}
// Rotation autour de X
void Camera::Pitch(double parAngle)
{
	FTransformation = FTransformation * glm::rotate(parAngle, glm::dvec3(1.0, 0.0, 0.0));
}
void Camera::UpdateValues(GLuint parShaderID)
{
	// Transformations
	const glm::dvec3& pos   = glm::dvec3(FTransformation[3]);
	const glm::dvec3& zAxis = glm::dvec3(FTransformation[2]);
	const glm::dvec3& yAxis = glm::dvec3(FTransformation[1]);
	const glm::dvec3& xAxis = glm::dvec3(FTransformation[0]);
	// Injection de la position
	ShaderManager::Instance().InjectVec3(parShaderID, pos, "cameraPosition");

	// Precalcules 
	const glm::dvec3& centreEcran = pos+zAxis*FLens;
	const glm::dvec3& coinSupGauche = centreEcran - yAxis * FHauteurEcran - xAxis * FHauteurEcran * RATIO;
	// Injection des données pour calculer les rayons primaires
	ShaderManager::Instance().InjectVec3(parShaderID, coinSupGauche, "coinSupGauche");
	ShaderManager::Instance().InjectVec3(parShaderID, xAxis * FHauteurEcran * 2.0, "unitX");
	ShaderManager::Instance().InjectVec3(parShaderID, yAxis * FHauteurEcran * 2.0, "unitY");
}
