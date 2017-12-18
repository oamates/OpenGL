#pragma once

#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../scene/texture.hpp"

#include "image/stb_image.h"
#include "log.hpp"


struct RawTexture;

struct TextureImporter
{
    static bool ImportTexture2D(const std::string& sFilepath, RawTexture& outTexture)
    {
        int width, height, bpp; // bytes per pixel

        unsigned char* data = stbi_load(sFilepath.c_str(), &width, &height, &bpp, 0);

        if (!data)
        {
            debug_msg("stbi :: failed to load image : %s", sFilepath.c_str());
            return false;
        }
        debug_msg("stbi :: image %s loaded.", sFilepath.c_str());

        outTexture.filepath = sFilepath;                                        // store data into texture class
        outTexture.height = height;
        outTexture.width = width;
        outTexture.depth = 1;
        outTexture.lineWidth = bpp * width;
        outTexture.bitsPerPixel = 8 * bpp * sizeof(unsigned char);
        outTexture.rawData.reset(data);
        return true;                                                            // successful image data extraction
    }
};

