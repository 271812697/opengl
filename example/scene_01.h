#pragma once

#include "../scene/scene.h"

namespace scene {

    class Scene01 : public Scene {
    public:

        using Scene::Scene;

      
        void Init() override;
        void OnSceneRender() override;
        void OnImGuiRender() override;



        Entity camera;
        Entity skybox;
        Entity direct_light;
        Entity orbit_light;
        Entity point_lights[28];

        Entity sphere;
        Entity plane;
        Entity runestone;
        Entity alien;

        asset_tmp<SSBO> pl_color;
        asset_tmp<SSBO> pl_position;
        asset_tmp<SSBO> pl_range;
        asset_tmp<SSBO> pl_index;

        asset_ref<Texture> irradiance_map;
        asset_ref<Texture> prefiltered_map;
        asset_ref<Texture> BRDF_LUT;

        void PrecomputeIBL(const std::string& hdri);
        void SetupMaterial(Material& pbr_mat, int mat_id);
        void SetupPLBuffers();
        void UpdatePLColors();
    };

}
