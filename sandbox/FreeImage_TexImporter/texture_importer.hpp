#pragma once

#include <string>
#include <cstdio>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <FreeImagePlus.h>

#include "../scene/texture.hpp"

struct RawTexture;

struct TextureImporter
{
    static bool ImportTexture2D(const std::string& sFilepath, RawTexture& outTexture)
    {
        FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(sFilepath.c_str());
        
        if (fif == FIF_UNKNOWN)                                                 // if still unknown, try to guess the file format from the file extension
            fif = FreeImage_GetFIFFromFilename(sFilepath.c_str());
        
        if (fif == FIF_UNKNOWN)                                                 // if still unkown, exit
            return false;
    
        FIBITMAP * dib = nullptr;                                               // pointer to the image once loaded
    
        if (FreeImage_FIFSupportsReading(fif))                                  // check that the plugin has reading capabilities and load the file
            dib = FreeImage_Load(fif, sFilepath.c_str());
    
        auto bits = FreeImage_GetBits(dib);                                     // get raw data
        auto width = FreeImage_GetWidth(dib);                                   // get image data
        auto height = FreeImage_GetHeight(dib);
        auto lineWidth = FreeImage_GetPitch(dib);
        auto bitsPerPixel = FreeImage_GetBPP(dib);
    
        if ((bitsPerPixel == 0) || (height == 0) || (width == 0) || !bits)      // If somehow one of these failed, return failure
        {
            FreeImage_Unload(dib);
            return false;
        }
        
        size_t buffer_size = height * lineWidth * sizeof(unsigned char);        // copy data before unload
        unsigned char* data = reinterpret_cast<unsigned char *> (malloc(buffer_size));
    
        if (!data)                                                              // couldn't allocate memory
        {
            FreeImage_Unload(dib);
            return false;
        }
    
        std::move(bits, bits + buffer_size, data);
        outTexture.filepath = sFilepath;                                        // store data into texture class
        outTexture.height = height;
        outTexture.width = width;
        outTexture.depth = 1;
        outTexture.lineWidth = lineWidth;
        outTexture.bitsPerPixel = bitsPerPixel;
        outTexture.rawData.reset(data);
        
        FreeImage_Unload(dib);                                                  // unload free image struct handler    
        return true;                                                            // successful image data extraction
    }
};

