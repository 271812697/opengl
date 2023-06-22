
#pragma once

#include "Scene.h"

namespace PathTrace
{
    class Scene;

    bool LoadSceneFromFile(const std::string& filename, Scene* scene, RenderOptions& renderOptions);
}