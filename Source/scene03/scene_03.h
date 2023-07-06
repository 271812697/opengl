#pragma once

#include "Opengl/scene/scene.h"

namespace scene {

    class Scene03 : public Scene {

        using Scene::Scene;

        void Init() override;
        void OnSceneRender(float dt = 0) override;
        void OnImGuiRender(float dt = 0) override;

        Entity camera;
        Entity skybox;
        Entity direct_light;

        Entity pistol;   // clear coat
        Entity helmet;   // anisotropy
        Entity pyramid;  // refraction (cubic)
        Entity capsule;  // refraction (spherical)

        asset_ref<Texture> irradiance_map;
        asset_ref<Texture> prefiltered_map;
        asset_ref<Texture> BRDF_LUT;

        void PrecomputeIBL(const std::string& hdri);
        void SetupMaterial(Material& mat);
        Entity& GetEntity(int entity_id);
    };

}
