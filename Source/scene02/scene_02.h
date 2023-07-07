#pragma once
#include"Opengl/scene/scene.h"
namespace scene{
    class Scene02 :public Scene {
        using Scene::Scene;
        void Init() override;
        void OnSceneRender(float dt = 0) override;
        void OnImGuiRender(float dt = 0) override;
        void Resize(int w, int h)override;
        void Present() override;

        Entity camera;
        Entity skybox;
        Entity point_light;
        Entity sphere[10];
        Entity cube[3];
        Entity torus;
        Entity motorbike;

        asset_ref<Texture>irradiance_map;
        asset_ref<Texture>prefiltered_map;
        asset_ref<Texture>BRDF_LUT;

        void PrecomputeIBL(const std::string& hdri);
        void SetupMaterial(Material& pbr_mat, int mat_id);

        void RenderSphere();
        void RenderTorus();
        void RenderCubes();
        void RenderMotor();

    };
}
