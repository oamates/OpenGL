
// .dat scene file parser

#ifndef PARSER_IO
#define PARSER_IO
#include <string>
#include <fstream>

#include "defines.hpp"
#include "scene.hpp"
#include "stringutils.hpp"
#include "primitive.hpp"
#include "vector3.hpp"
#include "vector4.hpp"

#define STRING_CHECKFIND(Buffer, String) (Buffer.find(String) != std::string::npos)

struct Parser
{
    Parser() {}
    ~Parser() {}
    Scene* GetSceneFromFile(std::string filename);
 
    enum EtatTraitementScene
    { 
        TRAITEMENT_SCENE,
        TRAITEMENT_LUMIERE,
        TRAITEMENT_TRIANGLE,
        TRAITEMENT_PLAN,
        TRAITEMENT_QUADRIQUE
    };

    const static int NB_MAX_CAR_PAR_LIGNE = 80;
};

#endif //PARSER_IO
