#ifndef COMMON_DEFINES
#define COMMON_DEFINES
#include <iostream>
#include <assert.h>

// Constantes utiles
namespace MathTools
{
    static const double PI = 3.14159265359;
    static const double FULL_ANGLE = 360.0;
}

// Taille de l'ecran
static int USCREEN_X = 1280;
static int USCREEN_Y = 720;
static double SCREEN_X = double(USCREEN_X);
static double SCREEN_Y = double(USCREEN_Y);
static double RATIO = SCREEN_X / SCREEN_Y;
 

// Conversion de degrés en radian
#define DegreeToRadian(Angle) (Angle*MathTools::PI/MathTools::FULL_ANGLE)
// Macro de prcours
#define foreach(IT,X) for ( typeof( X.begin() ) IT = X.begin(); IT != X.end(); ++IT)
#define tryget(IT,LIST,ELEM) typeof(LIST.begin()) IT = LIST.find(ELEM);

// Macro d'assert
#define AssertNotValid(stuff)  PRINT_RED(stuff); assert(false);

// File that includes opengl includes
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#endif //COMMON_DEFINES
