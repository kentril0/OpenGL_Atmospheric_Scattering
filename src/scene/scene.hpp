#pragma once

#include "../gui/gui.hpp"
#include "terrain.hpp"


/**
 * @brief Holds any necessary data for rendering a scene
 */
class Scene
{
public:
    Scene();

    
private:

    //std::unique_ptr<Airplane> player = nullptr;

    GUI* gui = nullptr;

    std::unique_ptr<Terrain> terrain = nullptr;
    //std::unique_ptr<Skybox> skybox   = nullptr;

    std::map<int, void (fnc*)(void)> key_bindings;
};
