/**********************************************************
 * < Interactive Atmospheric Scattering >
 * @author Martin Smutny, xsmutn13@stud.fit.vutbr.cz
 * @date April, 2021
 * @file utilities.hpp
 * @brief Utility functions, e.g. loading a file used 
 *        across modules
 *********************************************************/

#pragma once

#define CHANNELS_RGB 3
#define CHANNELS_RGBA 4


/**
 * @brief Loads a file to a string.
 * @param filename Path to a file.
 * @return Contents of the file as a string of chars.
 */
const std::string load_file(const char* filename);

/**
 * @brief Loads an image using STBI library.
 * @param filename Path to an image file.
 * @param alpha Whenther alpha channel is used. If true then RGBA is used, else
 *              based on param desired_channels.
 * @param width Returns the loaded image's width.
 * @param height Returns the loaded image's height.
 * @param channels Returns number of loaded image's channels.
 * @param desired_channels Load the image with a desired number of channels.
 *                         When 0 and alpha is false then RGB is used, else
 *                         any number without alpha is used.
 * @return Image data or NULL if failed to load. Free using 
 *         free_image_data(data) function.
 */
uint8_t* load_image(const char* filename, 
                    bool alpha,
                    int* width, int* height,
                    int* channels, 
                    int desired_channels = 0);

/**
 * @brief Frees the allocated data using STBI library (fnc load_image()).
 * @param data Data to be freed.
 */
void free_image_data(uint8_t* data);

