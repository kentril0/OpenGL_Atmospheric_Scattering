/**********************************************************
 * < Interative Atmospheric Scattering >
 * @author Martin Smutny, xsmutn13@stud.fit.vutbr.cz
 * @date April, 2021
 * @file pch.hpp
 * @brief Precompiled Header with used inclusions and 
 *        macro definitions
 *********************************************************/

#pragma once

#include <iostream>
#include <memory>

#include <string>
#include <vector>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>

#include <glad/glad.h>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext.hpp>

#include "utilities.hpp"

#define PROJECT_NAME "Interactive Atmospheric Scattering"

// TODO turn on debug outputs
#define DEBUG

// TODO enable asserts
//#define ENABLE_ASSERTS

// TODO define logging level
// Logging messages which are less severe than LOG_LEVEL will be ignored
#define LOG_LEVEL LEVEL_OK

#include "log.hpp"


// TODO OpenGL version
#define OPENGL_VERSION 45           // GCC compatible, only integral comparison
#define OPENGL_4_5


#if defined OPENGL_3_3
    #define OPENGL_VERSION_MAJOR 3
    #define OPENGL_VERSION_MINOR 3
    #define GLSL_VERSION_STR "#version 330"
#elif defined OPENGL_4_5
    #define OPENGL_VERSION_MAJOR 4
    #define OPENGL_VERSION_MINOR 5
    #define GLSL_VERSION_STR "#version 450"
#else 
    #define OPENGL_VERSION_MAJOR 3
    #define OPENGL_VERSION_MINOR 3
    #define GLSL_VERSION_STR "#version 330"
#endif

#define GLSL_PROFILE " core"

// TODO initial screen dimensions
#define SCREEN_INIT_WIDTH  1280
#define SCREEN_INIT_HEIGHT 720 


