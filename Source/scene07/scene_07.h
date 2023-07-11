#pragma once

#include "Opengl/scene/scene.h"

namespace scene {

    class Scene07 : public Scene {
    public:

        using Scene::Scene;
        void Init() override;
        void OnSceneRender(float dt = 0) override;
        void OnImGuiRender(float dt = 0) override;
        void Resize(int w, int h)override;
        void Present() override;

        asset_ref<Texture> irradiance_map;
        asset_ref<Texture> prefiltered_map;
        asset_ref<Texture> BRDF_LUT;

        Entity camera;
        Entity skybox;
        Entity point_light;
        Entity spotlight;
        Entity floor;
        Entity wall;
        Entity ball[3];
        void PrecomputeIBL(const std::string& hdri);
        void SetupMaterial(Material& mat, int id);
        
    };

}
