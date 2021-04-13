################################################################################
# Project: Interactive Atmospheric Scattering
# Author: Martin Smutny, xsmutn13@stud.fit.vutbr.cz
# Date: April, 2021
#
# Project directory tree (approx., may not be up to date):
# ./
#  |-- src/
#  |   |-- core/
#  |   |-- libs/
#  |   |   |-- glad/
#  |   |   |-- glfw/
#  |   |   |-- single_header/
#  |   |-- opengl/
#  |   |-- scene/
#  |   |-- shaders/
#
################################################################################

cmake_minimum_required(VERSION 3.16)

# Project name
project(demo)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(UNIX)
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED)
    find_package(Threads REQUIRED)
endif()

#-------------------------------------------------------------------------------
# Set Directories
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
    set(SRC_CORE_DIR    "${SRC_DIR}/core")
    set(LIBRARIES_DIR   "${SRC_DIR}/libs")
    set(SRC_OPENGL_DIR  "${SRC_DIR}/opengl")
    set(SRC_SCENE_DIR   "${SRC_DIR}/scene")
    set(SRC_SHADERS_DIR "${SRC_DIR}/shaders")
set(OBJECTS_DIR "${CMAKE_SOURCE_DIR}/objects")


#-------------------------------------------------------------------------------
# EXTERNAL LIBRARIES:
#   GLFW
#   GLAD
#   Single Header (glm, stbi, ...)
#-------------------------------------------------------------------------------

include(ExternalProject)

#-------------------------------------------------------------------------------
# GLFW library
set(GLFW_INSTALL_DIR "${CMAKE_BINARY_DIR}/glfw")
set(GLFW_INCLUDE_DIR "${LIBRARIES_DIR}/glfw/include")
set(GLFW_LIBRARIES   "${GLFW_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}glfw3${CMAKE_STATIC_LIBRARY_SUFFIX}")

ExternalProject_Add(glfw
  SOURCE_DIR "${LIBRARIES_DIR}/glfw"

  DOWNLOAD_COMMAND ""
  UPDATE_COMMAND ""

  CMAKE_ARGS
      "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/glfw"
      "-DGLFW_BUILD_EXAMPLES=OFF"
      "-DGLFW_BUILD_TESTS=OFF"
      "-DGLFW_BUILD_DOCS=OFF"
      "-DBUILD_SHARED_LIBS=OFF"

  BUILD_BYPRODUCTS ${GLFW_LIBRARIES}
)

if(UNIX)
    set(GLFW_LIBRARIES ${GLFW_LIBRARIES} ${OPENGL_LIBRARIES} ${X11_LIBRARIES} 
        ${X11_Xrandr_LIB} ${X11_Xcursor_LIB} ${X11_Xinerama_LIB} ${X11_Xxf86vm_LIB}
        ${CMAKE_THREAD_LIBS_INIT})
endif()


#-------------------------------------------------------------------------------
# GLAD library
set(GLAD_INCLUDE_DIR "${LIBRARIES_DIR}/glad/include")

add_library(glad "${GLAD_INCLUDE_DIR}/glad/glad.h" "${LIBRARIES_DIR}/glad/src/glad.c")
target_include_directories(glad PUBLIC "${GLAD_INCLUDE_DIR}")

set(GLAD_LIBARIES)
if(UNIX)
    set(GLAD_LIBRARIES "glad" ${CMAKE_DL_LIBS})
else()
    set(GLAD_LIBRARIES glad)
endif()

#--------------------------------------------------------------------------------
# Single header libraries
set(SINGLE_HEADER_LIBS_INCLUDE_DIR "${LIBRARIES_DIR}/single_header")

if (CMAKE_CXX_COMPILER_ID MATCHES Clang OR ${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
    # using regular Clang or AppleClang or GCC
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    # using Visual Studio C++
    set(CMAKE_CXX_FLAGS "/std:c++17 /permissive- /W3 /EHsc")
endif()


#--------------------------------------------------------------------------------
# imGUI library
set(IMGUI_INCLUDE_DIR "${LIBRARIES_DIR}/imgui")

add_library(imgui 
    "${IMGUI_INCLUDE_DIR}/imgui.h"
    "${IMGUI_INCLUDE_DIR}/imgui.cpp"
    "${IMGUI_INCLUDE_DIR}/imconfig.h"
    "${IMGUI_INCLUDE_DIR}/imgui_internal.h"
    "${IMGUI_INCLUDE_DIR}/imstb_textedit.h"
    "${IMGUI_INCLUDE_DIR}/imstb_rectpack.h"
    "${IMGUI_INCLUDE_DIR}/imstb_truetype.h"
    "${IMGUI_INCLUDE_DIR}/imgui_widgets.cpp"
    "${IMGUI_INCLUDE_DIR}/imgui_demo.cpp"
    "${IMGUI_INCLUDE_DIR}/imgui_draw.cpp"
    "${IMGUI_INCLUDE_DIR}/imgui_impl_glfw.h"
    "${IMGUI_INCLUDE_DIR}/imgui_impl_glfw.cpp"
    "${IMGUI_INCLUDE_DIR}/imgui_impl_opengl3.h"
    "${IMGUI_INCLUDE_DIR}/imgui_impl_opengl3.cpp"
)
add_dependencies(imgui glfw glad)
target_link_libraries(imgui
    ${GLFW_LIBRARIES}
    ${GLAD_LIBRARIES}
)

target_include_directories(imgui 
    PRIVATE ${GLFW_INCLUDE_DIR}
    PRIVATE ${GLAD_INCLUDE_DIR}
    PUBLIC ${IMGUI_INCLUDE_DIR}
)

target_compile_options(imgui
  PRIVATE "-DIMGUI_IMPL_OPENGL_LOADER_GLAD"
)


#--------------------------------------------------------------------------------
# Project
#--------------------------------------------------------------------------------
# TODO glob??
set(sources 
    "${SRC_CORE_DIR}/main.cpp"
    "${SRC_CORE_DIR}/application.cpp"
    "${SRC_CORE_DIR}/utilities.cpp"
    "${SRC_OPENGL_DIR}/buffer.cpp"
    "${SRC_OPENGL_DIR}/shader.cpp"
    "${SRC_OPENGL_DIR}/texture2d.cpp"
    "${SRC_OPENGL_DIR}/vertex_array.cpp"
    "${SRC_SCENE_DIR}/camera.cpp"
    "${SRC_SCENE_DIR}/mesh.cpp"
)

#--------------------------------------------------------------------------------
# TODO for Windows 10 standalone exe

if(WIN32)
    set(CMAKE_CXX_STANDARD_LIBRARIES 
       "-static-libgcc -static-libstdc++ -lwsock32 -lws2_32 
       ${CMAKE_CXX_STANDARD_LIBRARIES}")

    set(CMAKE_EXE_LINKER_FLAGS 
       "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive")
endif()

add_executable(${CMAKE_PROJECT_NAME} ${sources})

add_dependencies(${CMAKE_PROJECT_NAME}
    glfw
    glad
    imgui
)

#set (CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")

target_precompile_headers(${CMAKE_PROJECT_NAME}
    PUBLIC "${SRC_CORE_DIR}/pch.hpp" 
)

target_link_libraries(${CMAKE_PROJECT_NAME}
    ${GLFW_LIBRARIES}
    ${GLAD_LIBRARIES}
    imgui
)

target_include_directories(${CMAKE_PROJECT_NAME}
    PRIVATE ${GLFW_INCLUDE_DIR}
    PRIVATE ${GLAD_INCLUDE_DIR}
    PRIVATE ${SINGLE_HEADER_LIBS_INCLUDE_DIR}
    PRIVATE ${IMGUI_INCLUDE_DIR}
    PRIVATE ${SRC_DIR}
)


#--------------------------------------------------------------------------------
# Copy assets to build folder
#--------------------------------------------------------------------------------

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${SRC_SHADERS_DIR}
    ${PROJECT_BINARY_DIR}/shaders
    COMMENT "Copy shaders to build tree")

# TODO maybe a bad idea
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${OBJECTS_DIR}
    ${PROJECT_BINARY_DIR}/objects
    COMMENT "Copy objects to build tree")

