# Atmospheric Scattering in OpenGL
Atmospheric scattering written in C++ using OpenGL

## Tools:
* cmake >=3.16
* OpenGL >=3.3 compatible hardware
* C++17 compiler

## Libraries:
* GLFW3
* GLAD
* GLM
* STBI


## How to compile and run

$ mkdir build
$ cd build
$ cmake ..
$ make
$ ./app


## Common issues

### Linux: CMake X11_Xxf86vm_LIB error
Probably need to install packages:
    libxss-dev libxxf86vm-dev libxkbfile-dev libxv-dev


## TODO
rewrite file headers

compile for different versions of OpenGL
setup logfiles - LOG_ERR fncs
links to libraries
