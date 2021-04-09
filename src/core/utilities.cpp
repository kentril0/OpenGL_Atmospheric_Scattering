/**********************************************************
 * < Interactive Atmospheric Scattering >
 * @author Martin Smutny, xsmutn13@stud.fit.vutbr.cz
 * @date April, 2021
 * @file utilities.cpp
 * @brief Utility functions, e.g. loading a file used 
 *        across modules
 *********************************************************/

#include "pch.hpp"
#include "utilities.hpp"

#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


const std::string load_file(const char* filename)
{
    std::ifstream infile{filename};

    if (!infile)
    {
        LOG_ERR("File " << filename << " does not exist");
        throw "File does not exist";
    }

    return { std::istreambuf_iterator<char>(infile), 
             std::istreambuf_iterator<char>() };
}


uint8_t* load_image(const char* filename, bool alpha,
                    int* width, int* height,
                    int* channels, int desired_channels)
{
    uint8_t* data = stbi_load(filename, width, height, channels,
                              (alpha == true ? CHANNELS_RGBA :
                                     (desired_channels == 0 ? CHANNELS_RGB :
                                                              desired_channels)
                              ));
    return data;
}

void free_image_data(uint8_t* data)
{
    stbi_image_free(data);
}

