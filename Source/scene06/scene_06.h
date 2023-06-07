#pragma once

#include "Opengl/scene/scene.h"

namespace scene {

    class Scene06 : public Scene {
    public:

        using Scene::Scene;

      
        void Init() override;
        void OnSceneRender() override;
        void OnImGuiRender() override;
        ~Scene06()override;
    };

}
