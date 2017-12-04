#ifndef CAMERA
#define CAMERA

#include <glm/glm.hpp>

#include "defines.hpp"

struct Camera
{
    // Constructeur
    Camera();
    // Destructeur
    ~Camera();
    // méthode de rotation
    void Yaw(double parAngle);
    void Pitch(double parAngle);
    // Methode de translation
    void Translate(const glm::dvec3& parDir);
    // Mise a jour des valeurs dans un programme
    void UpdateValues(GLuint parShaderID);

    // Transformation
    glm::dmat4 FTransformation;
    // Informations de perspective
    double FLens;
    double FAngleView;
    double FHauteurEcran;
    double FPasX;
    double FPasY;

};

#endif //CAMERA
