
#pragma once

#include "Scene.h"

namespace PathTrace
{
    class Scene;
    bool LoadGLTF(const std::string& filename, Scene* scene, RenderOptions& renderOptions, Mat4 xform, bool binary);
    bool LoadSceneFromFile(const std::string& filename, Scene* scene, RenderOptions& renderOptions);
}