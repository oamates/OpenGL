#pragma once

#include <string>

struct RawTexture;

struct TextureImporter
{
    static bool ImportTexture2D(const std::string &sFilepath, RawTexture &outTexture);
};

