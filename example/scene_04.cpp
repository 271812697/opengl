#include "../pch.h"

#include "../core/base.h"
#include "../core/clock.h"
#include "../core/input.h"
#include "../core/window.h"
#include "../core/sync.h"
#include "../asset/all.h"
#include "../component/all.h"
#include "../scene/renderer.h"
#include "../scene/ui.h"
#include "../util/ext.h"
#include "../util/math.h"
#include "../util/path.h"
#include"scene_04.h"
using namespace core;
using namespace asset;
using namespace component;
using namespace utils;
namespace scene {
    static float skybox_exposure = 1.0f;
    static float skybox_lod = 0.0f;
    static vec3  dl_direction = vec3(0.7f, -0.7f, 0.0f);
    void Scene04::Init() {
        Renderer::SetScene(this);
        PrecomputeIBL("res\\texture\\HDRI\\Field-Path-Fence-Steinbacher-Street-4K.hdr");
        this->title = "Animation";
        resource_manager.Add(02,MakeAsset<Shader>("skybox.glsl"));
        resource_manager.Add(04, MakeAsset<Material>(resource_manager.Get<Shader>(02)));
        resource_manager.Add(03, MakeAsset<Shader>("post_process04.glsl"));
        resource_manager.Add(05, MakeAsset<Shader>("pbr04.glsl"));
        resource_manager.Add(06, MakeAsset<Material>(resource_manager.Get<Shader>(05)));
        resource_manager.Add(07, MakeAsset<Shader>("light.glsl"));
        resource_manager.Add(8, MakeAsset<Material>(resource_manager.Get<Shader>(07)));
        resource_manager.Add(9, MakeAsset<CShader>("bloom.glsl"));
        skybox = CreateEntity("sky", component::ETag::Skybox);
        skybox.AddComponent<Mesh>(Primitive::Cube);
        if (auto& mat = skybox.AddComponent<Material>(resource_manager.Get<Material>(04)); true) {
            mat.SetTexture(0, prefiltered_map);
            mat.BindUniform(0, &skybox_exposure);
            mat.BindUniform(1, &skybox_lod);
        }
        direct_light = CreateEntity("Directional Light");
        direct_light.GetComponent<Transform>().Rotate(45.0f, 180.0f, 0.0f, Space::World);
        direct_light.AddComponent<DirectionLight>(color::yellow, 0.2f);  // 0 attenuation -> small intensity
        

        point_light = CreateEntity("Point Light");
        point_light.AddComponent<Mesh>(Primitive::Sphere);
        point_light.GetComponent<Transform>().Translate(world::up * 6.0f);
        point_light.GetComponent<Transform>().Translate(world::backward * 4.0f);
        point_light.GetComponent<Transform>().Scale(0.1f);
        point_light.AddComponent<PointLight>(color::white, 1.8f);
        point_light.GetComponent<PointLight>().SetAttenuation(0.09f, 0.032f);
        if (auto& mat=point_light.AddComponent<Material>(resource_manager.Get<Material>(8)); true) {
            auto& pl = point_light.GetComponent<PointLight>();
            mat.SetUniform(3, pl.color);
            mat.SetUniform(4, pl.intensity);
            mat.SetUniform(5, 4.0f);
        
        }
        
        
        camera= CreateEntity("camera", component::ETag::MainCamera);
        camera.GetComponent<Transform>().Translate(0.0f, 6.0f, 9.0f);
        camera.AddComponent<Camera>(View::Perspective);
        mingyue= CreateEntity("mingyue", component::ETag::Untagged);
        
        mingyue.GetComponent<Transform>().Scale(2.5f);
        mingyue.GetComponent<Transform>().Translate(vec3(5.0f, 0.0f, 5.0f));   
        Model& model=mingyue.AddComponent<Model>("res/Zhaolinger/Hero_Zhaolinger/Hero_Zhaolinger.fbx",component::Quality::High, true);
        mingyue.AddComponent<Animator>();
        SetupMaterial(model.SetMaterial("Hero_Zhaolinger_Body_Mat_Show",resource_manager.Get<Material>(06)),1);
        SetupMaterial(model.SetMaterial("Hero_Zhaolinger_Hair_Mat_Show",resource_manager.Get<Material>(06)), 2);
        SetupMaterial(model.SetMaterial("Hero_Zhaolinger_Face_Mat_Show",resource_manager.Get<Material>(06)), 3);
        
        zhaolinger= CreateEntity("zhaolinger", component::ETag::Untagged);
        zhaolinger.GetComponent<Transform>().Scale(2.5f);
        zhaolinger.GetComponent<Transform>().Translate(vec3(10.0f, 0.0f, 5.0f));

        Model& modelz = zhaolinger.AddComponent<Model>("res/mingyue/Hero_Mingyue/Hero_Mingyue.fbx", component::Quality::High, true);
        zhaolinger.AddComponent<Animator>();
        SetupMaterial(modelz.SetMaterial("Hero_Mingyue_Hair", resource_manager.Get<Material>(06)), 4);
        SetupMaterial(modelz.SetMaterial("Hero_Mingyue_Body_sha", resource_manager.Get<Material>(06)), 5);
        SetupMaterial(modelz.SetMaterial("Hero_Mingyue_Face", resource_manager.Get<Material>(06)), 6);
        SetupMaterial(modelz.SetMaterial("Hero_Mingyue_Body", resource_manager.Get<Material>(06)), 7);

        
        //UBO
        AddUBO(resource_manager.Get<Shader>(02)->ID());
        AddUBO(resource_manager.Get<Shader>(05)->ID());
        //FBO
        AddFBO(Window::width, Window::height);
        AddFBO(Window::width, Window::height);
        AddFBO(Window::width / 2, Window::height / 2);
        FBOs[0].AddColorTexture(2, true);    // multisampled textures for MSAA
        FBOs[0].AddDepStRenderBuffer(true);  // multisampled RBO for MSAA
        FBOs[1].AddColorTexture(2);
        FBOs[2].AddColorTexture(2);
        //RenderState
        Renderer::MSAA(true);
        Renderer::DepthTest(true);
        Renderer::AlphaBlend(true);
        Renderer::FaceCulling(true);
    }
    void Scene04::OnSceneRender() {
        //
        auto& animator=mingyue.GetComponent<Animator>();
        auto& animatorz = zhaolinger.GetComponent<Animator>();
        animator.Update(Clock::delta_time);
        animatorz.Update(Clock::delta_time);
        //UBO
        auto& main_camera = camera.GetComponent<Camera>();
        main_camera.Update();
        if (auto& ubo = UBOs[0]; true) {
            ubo.SetUniform(0, val_ptr(main_camera.T->position));
            ubo.SetUniform(1, val_ptr(main_camera.T->forward));
            ubo.SetUniform(2, val_ptr(main_camera.GetViewMatrix()));
            ubo.SetUniform(3, val_ptr(main_camera.GetProjectionMatrix()));
        }
        if (auto& ubo = UBOs[1]; true) {
            auto& dl = direct_light.GetComponent<DirectionLight>();
            vec3 direction = -glm::normalize(dl_direction);
            ubo.SetUniform(0, val_ptr(dl.color));
            ubo.SetUniform(1, val_ptr(direction));
            ubo.SetUniform(2, val_ptr(dl.intensity));
        }
        if (auto& ubo = UBOs[3]; true) {
            auto& pl = point_light.GetComponent<PointLight>();
            auto& pt = point_light.GetComponent<Transform>();
            ubo.SetUniform(0, val_ptr(pl.color));
            ubo.SetUniform(1, val_ptr(pt.position));
            ubo.SetUniform(2, val_ptr(pl.intensity));
            ubo.SetUniform(3, val_ptr(pl.linear));
            ubo.SetUniform(4, val_ptr(pl.quadratic));
            ubo.SetUniform(5, val_ptr(pl.range));
        }
        //MRT
        FBO& framebuffer_0 = FBOs[0];
        FBO& framebuffer_1 = FBOs[1];
        FBO& framebuffer_2 = FBOs[2];
        framebuffer_0.Clear();
        framebuffer_0.Bind();
        Renderer::Submit(skybox.id);   
        Renderer::Submit(mingyue.id);
        Renderer::Submit(zhaolinger.id);
        Renderer::Submit(point_light.id);
        Renderer::Render();
        framebuffer_0.Unbind();
        //MSAA
        framebuffer_1.Clear();
        FBO::CopyColor(framebuffer_0, 0, framebuffer_1, 0);
        FBO::CopyColor(framebuffer_0, 1, framebuffer_1, 1);
        //post_process
        FBO::CopyColor(framebuffer_1, 1, framebuffer_2, 0);  // downsample the bloom target (nearest filtering)
        auto& ping = framebuffer_2.GetColorTexture(0);
        auto& pong = framebuffer_2.GetColorTexture(1);
        auto bloom_shader = resource_manager.Get<CShader>(9);

        bloom_shader->Bind();
        ping.BindILS(0, 0, GL_READ_WRITE);
        pong.BindILS(0, 1, GL_READ_WRITE);

        for (int i = 0; i < 6; ++i) {
            bloom_shader->SetUniform(0, i % 2 == 0);
            bloom_shader->Dispatch(ping.width / 32, ping.height/ 18);
            bloom_shader->SyncWait(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
        }

        auto postprocess_shader = resource_manager.Get<Shader>(03);
        postprocess_shader->Bind();
        framebuffer_1.GetColorTexture(0).Bind(0);
        framebuffer_2.GetColorTexture(1).Bind(1);
        postprocess_shader->SetUniform(0,3);
        Renderer::Clear();
        Mesh::DrawQuad();
    }
    void Scene04::OnImGuiRender() {
        using namespace ImGui;
        if (ui::NewInspector()) {
            Indent(5.0f);
            PushItemWidth(130.0f);
            SliderFloat("Skybox Exposure", &skybox_exposure, 0.5f, 2.0f);
            SliderFloat("Skybox LOD", &skybox_lod, 0.0f, 7.0f);
            PopItemWidth();
            Separator();
            {

            static int flag = 0;
            static int cur_ani = 0;
            static int cnt_ani = 0;
            static char** item = nullptr;
            if (!flag) {
                flag = 1;
                auto& model=mingyue.GetComponent<Model>(); 
                cnt_ani=model.animations.size();
                item = new  char* [cnt_ani];
                for (int i = 0; i < cnt_ani; i++) {
                    item[i] = new  char[100];
                    strcpy(item[i],model.animations[i]->name.c_str());
                }
            }
            if (Combo("Animations", &cur_ani, item, cnt_ani)) {
                auto& animator = mingyue.GetComponent<Animator>();
                animator.cur_animation=cur_ani;
            }



            }
            {

                static int flag = 0;
                static int cur_ani = 0;
                static int cnt_ani = 0;
                static char** item = nullptr;
                if (!flag) {
                    flag = 1;
                    auto& model = zhaolinger.GetComponent<Model>();
                    cnt_ani = model.animations.size();
                    item = new  char* [cnt_ani];
                    for (int i = 0; i < cnt_ani; i++) {
                        item[i] = new  char[100];
                        strcpy(item[i], model.animations[i]->name.c_str());
                    }
                }
                if (Combo("AnimationsZ", &cur_ani, item, cnt_ani)) {
                    auto& animator = zhaolinger.GetComponent<Animator>();
                    animator.cur_animation = cur_ani;
                }



            }
            ui::DrawGizmo(camera, point_light, ui::Gizmo::Translate);
            ui::EndInspector();
        }
    }
    void Scene04::PrecomputeIBL(const std::string& hdri)
    {
        Renderer::SeamlessCubemap(true);
        Renderer::DepthTest(false);
        Renderer::FaceCulling(true);

        auto irradiance_shader = CShader("irradiance_map.glsl");
        auto prefilter_shader = CShader("prefilter_envmap.glsl");
        auto envBRDF_shader = CShader("environment_BRDF.glsl");

        auto env_map = MakeAsset<Texture>(hdri, 2048, 0);
        env_map->Bind(0);

        irradiance_map = MakeAsset<Texture>(GL_TEXTURE_CUBE_MAP, 128, 128, 6, GL_RGBA16F, 1);
        prefiltered_map = MakeAsset<Texture>(GL_TEXTURE_CUBE_MAP, 2048, 2048, 6, GL_RGBA16F, 8);
        BRDF_LUT = MakeAsset<Texture>(GL_TEXTURE_2D, 1024, 1024, 1, GL_RGBA16F, 1);

        CORE_INFO("Precomputing diffuse irradiance map from {0}", hdri);
        irradiance_map->BindILS(0, 0, GL_WRITE_ONLY);

        if (irradiance_shader.Bind(); true) {
            irradiance_shader.Dispatch(128 / 32, 128 / 32, 6);
            irradiance_shader.SyncWait(GL_TEXTURE_FETCH_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);

            auto irradiance_fence = Sync(0);
            irradiance_fence.ClientWaitSync();
            irradiance_map->UnbindILS(0);
        }

        CORE_INFO("Precomputing specular prefiltered envmap from {0}", hdri);
        Texture::Copy(*env_map, 0, *prefiltered_map, 0);  // copy the base level

        const GLuint max_level = prefiltered_map->n_levels - 1;
        GLuint resolution = prefiltered_map->width / 2;
        prefilter_shader.Bind();

        for (unsigned int level = 1; level <= max_level; level++, resolution /= 2) {
            float roughness = level / static_cast<float>(max_level);
            GLuint n_groups = glm::max<GLuint>(resolution / 32, 1);

            prefiltered_map->BindILS(level, 1, GL_WRITE_ONLY);
            prefilter_shader.SetUniform(0, roughness);
            prefilter_shader.Dispatch(n_groups, n_groups, 6);
            prefilter_shader.SyncWait(GL_TEXTURE_FETCH_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);

            auto prefilter_fence = Sync(level);
            prefilter_fence.ClientWaitSync();
            prefiltered_map->UnbindILS(1);
        }

        CORE_INFO("Precomputing specular environment BRDF from {0}", hdri);
        BRDF_LUT->BindILS(0, 2, GL_WRITE_ONLY);

        if (envBRDF_shader.Bind(); true) {
            envBRDF_shader.Dispatch(1024 / 32, 1024 / 32, 1);
            envBRDF_shader.SyncWait(GL_ALL_BARRIER_BITS);
            Sync::WaitFinish();
            BRDF_LUT->UnbindILS(2);
        }
    }
    void Scene04::SetupMaterial(Material& mat,int id)
    {
        mat.SetTexture(pbr_t::irradiance_map, irradiance_map);
        mat.SetTexture(pbr_t::prefiltered_map, prefiltered_map);
        mat.SetTexture(pbr_t::BRDF_LUT, BRDF_LUT);
        mat.BindUniform(0, &skybox_exposure);
        if (id == 1) {
            //body
           // mat.SetTexture(pbr_t::albedo, MakeAsset<Texture>("C:\\Users\\271812697\\Desktop\\donghua\\Zhaolinger\\Hero_Zhaolinger\\Hero_Zhaolinger_Body.png"));
            auto& bone_transforms = mingyue.GetComponent<Animator>().bone_transforms;

            mat.SetUniformArray(100U, bone_transforms.size(), &bone_transforms);
        }
        else if (id == 2) {
            //hair
           // mat.SetTexture(pbr_t::albedo, MakeAsset<Texture>("C:\\Users\\271812697\\Desktop\\donghua\\Zhaolinger\\Hero_Zhaolinger\\Hero_Zhaolinger_Hair.png"));
            auto& bone_transforms = mingyue.GetComponent<Animator>().bone_transforms;

            mat.SetUniformArray(100U, bone_transforms.size(), &bone_transforms);
        
        }
        else if (id == 3) {
            //face
            auto& bone_transforms = mingyue.GetComponent<Animator>().bone_transforms;

            mat.SetUniformArray(100U, bone_transforms.size(), &bone_transforms);
            //mat.SetTexture(pbr_t::albedo, MakeAsset<Texture>("C:\\Users\\271812697\\Desktop\\donghua\\Zhaolinger\\Hero_Zhaolinger\\Hero_Zhaolinger_Face.png"));
        }
        else if (id == 4) {
            auto& bone_transforms = zhaolinger.GetComponent<Animator>().bone_transforms;

            mat.SetUniformArray(100U, bone_transforms.size(), &bone_transforms);
            //face
           // mat.SetTexture(pbr_t::albedo, MakeAsset<Texture>("C:\\Users\\271812697\\Desktop\\donghua\\mingyue\\Hero_Mingyue_Show\\Hero_Mingyue_Hair.png"));
        }
        else if (id == 5) {
            auto& bone_transforms = zhaolinger.GetComponent<Animator>().bone_transforms;

            mat.SetUniformArray(100U, bone_transforms.size(), &bone_transforms);
            //face
           // mat.SetTexture(pbr_t::albedo, MakeAsset<Texture>("C:\\Users\\271812697\\Desktop\\donghua\\mingyue\\Hero_Mingyue_Show\\Hero_Mingyue_Body_sha.png"));
        }
        else if (id == 6) {
            auto& bone_transforms = zhaolinger.GetComponent<Animator>().bone_transforms;

            mat.SetUniformArray(100U, bone_transforms.size(), &bone_transforms);
            //face
           // mat.SetTexture(pbr_t::albedo, MakeAsset<Texture>("C:\\Users\\271812697\\Desktop\\donghua\\mingyue\\Hero_Mingyue_Show\\Hero_Mingyue_Face.png"));
        }
        else if (id == 7) {
            auto& bone_transforms = zhaolinger.GetComponent<Animator>().bone_transforms;

            mat.SetUniformArray(100U, bone_transforms.size(), &bone_transforms);
            //face
            //mat.SetTexture(pbr_t::albedo, MakeAsset<Texture>("C:\\Users\\271812697\\Desktop\\donghua\\mingyue\\Hero_Mingyue_Show\\Hero_Mingyue_Body.png"));
        }
        


    }
}