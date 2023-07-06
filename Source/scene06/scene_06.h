#pragma once

#include "Opengl/scene/scene.h"

namespace scene {

    class Scene06 : public Scene {
    public:

        using Scene::Scene;

      
        void Init() override;
        void UpdateScene(float dt = 0) override;
        void OnSceneRender(float dt=0) override;
        void OnImGuiRender(float dt=0) override;
        void Resize(int w, int h) override;
        void Present()override;
        ~Scene06()override;
        
    };

}
