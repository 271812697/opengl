#include "scene_07.h"

//主程序的全局数据
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <stdio.h>
#include<iostream>
#include<glad/glad.h>
#include <GLFW/glfw3.h>
#include"Window/Device.h"
#include"Window/WindowSettings.h"
#include"Window/Window.h"
#include"Window/InputManager.h"
#include"UI/Core/UIManager.h"
#include"UI/Styling/EStyle.h"
#include"tools/Clock.h"
#include"UI/Panels/PanelsManager.h"
#include"UI/Panels/AView.h"
#include"UI/Panels/Inspector.h"
#include"UI/Widgets/CustomWidget.h"
#include"Opengl/core/log.h"
#include"Opengl/core/clock.h"
#include"ImGuizmo.h"
#include<string>

Windowing::Settings::WindowSettings windowSettings;
Windowing::Settings::DeviceSettings deviceSettings;
std::unique_ptr<Windowing::Context::Device>	device;
std::unique_ptr<Windowing::Window> window;
std::unique_ptr<Windowing::Inputs::InputManager>inputManager;
std::unique_ptr<UI::Core::UIManager>uiManager;
UI::Settings::PanelWindowSettings settings;
std::unique_ptr<UI::Panels::PanelsManager>m_panelsManager;
using namespace std;

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw  %d: %s\n", error, description);
}
int main(int, char**) {
    {

        deviceSettings.contextMajorVersion = 4;
        deviceSettings.contextMinorVersion = 6;
        windowSettings.title = "PCSS Shadow and Animation";
        windowSettings.width = 1600;
        windowSettings.height = 900;
        windowSettings.maximized = true;
        device = std::make_unique<Windowing::Context::Device>(deviceSettings);
        window = std::make_unique<Windowing::Window>(*device, windowSettings);
        window->SetIcon("res/texture/awesomeface.png");
        inputManager = std::make_unique<Windowing::Inputs::InputManager>(*window);;
        window->MakeCurrentContext();

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            return -1;
        }
        device->SetVsync(true);
        //初始化场景
        ::core::Log::Init();
        scene::Scene* S = new scene::Scene07("PCSS Shadow and Animation");
        S->Init();
        S->Resize(1600, 900);
        //初始化UI
        uiManager = std::make_unique<UI::Core::UIManager>(window->GetGlfwWindow(), UI::Styling::EStyle::CUSTOM);;
        uiManager->LoadFont("Ruda_Big", "res/font/Ruda-Bold.ttf", 18);
        uiManager->LoadFont("Ruda_Small", "res/font/Ruda-Bold.ttf", 12);
        uiManager->LoadFont("Ruda_Medium", "res/font/Ruda-Bold.ttf", 14);
        uiManager->UseFont("Ruda_Big");
        uiManager->SetEditorLayoutSaveFilename(std::string(getenv("APPDATA")) + "\\layout.ini");
        uiManager->SetEditorLayoutAutosaveFrequency(60.0f);
        uiManager->EnableEditorLayoutSave(true);
        uiManager->EnableDocking(true);

        settings.closable = true;
        settings.collapsable = true;
        settings.dockable = true;
        UI::Modules::Canvas m_canvas;
        m_panelsManager = std::make_unique<UI::Panels::PanelsManager>(m_canvas);


        Tools::Time::Clock clock;
        m_panelsManager->CreatePanel<UI::Panels::MenuBar>("Menu Bar");
        m_panelsManager->CreatePanel<UI::Panels::Inspector>("Inspector", true, settings);
        m_panelsManager->GetPanelAs<UI::Panels::Inspector>("Inspector").CreateWidget<UI::Widgets::CustomWidget>().DrawIn += [&S, &clock]() {
            S->OnImGuiRender(clock.GetDeltaTime());
        };
        m_panelsManager->CreatePanel<UI::Panels::AView>("Scene View", true, settings);
        m_panelsManager->GetPanelAs<UI::Panels::AView>("Scene View").ResizeEvent += [&S](int p_width, int p_height) {
            S->Resize(p_width, p_height);
        };
        m_panelsManager->GetPanelAs<UI::Panels::AView>("Scene View").DrawInWindow += [S]() {
            if (S->selected_entity != -1) {
                ImGuizmo::MODE mode = ImGuizmo::MODE::LOCAL;
                ImGuizmo::OPERATION operation = ImGuizmo::OPERATION::TRANSLATE;
                if (S->cur_operation == 1)operation = ImGuizmo::OPERATION::ROTATE;
                if (S->cur_operation == 2)operation = ImGuizmo::OPERATION::SCALE;
                auto& T = S->directory_Entity[S->selected_entity].GetComponent<Transform>();
                auto& C = S->directory["Camera"].GetComponent<Camera>();
                glm::mat4 V = C.GetViewMatrix();
                glm::mat4 P = C.GetProjectionMatrix();
                auto pos = m_panelsManager->GetPanelAs<UI::Panels::AView>("Scene View").GetPosition();
                auto size = m_panelsManager->GetPanelAs<UI::Panels::AView>("Scene View").GetSize();
                // convert model matrix to left-handed as ImGuizmo assumes a left-handed coordinate system
                static const glm::vec3 RvL = glm::vec3(1.0f, 1.0f, -1.0f);  // scaling vec for R2L and L2R
                glm::mat4 transform = glm::scale(T.transform, RvL);
                ImGuizmo::SetRect(pos.x, pos.y, size.x, size.y);
                ImGuizmo::SetOrthographic(true);
                ImGuizmo::SetDrawlist();
                ImGuizmo::Manipulate(value_ptr(V), value_ptr(P), operation, mode, value_ptr(transform));
                if (ImGuizmo::IsUsing()) {
                    transform = glm::scale(transform, RvL);  // convert back to right-handed
                    T.SetTransform(transform);
                }
            }
        };
        m_canvas.MakeDockspace(true);
        uiManager->SetCanvas(m_canvas);
        //主循环
        while (!window->ShouldClose())
        {
            //glClearColor(0., 0., 0., 0.);
            //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
           // glDisable(GL_DEPTH_TEST);
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            S->UpdateScene(clock.GetDeltaTime());
            S->OnSceneRender(clock.GetDeltaTime());
            m_panelsManager->GetPanelAs<UI::Panels::AView>("Scene View").Update(1);
            m_panelsManager->GetPanelAs<UI::Panels::AView>("Scene View").Bind();
            S->Present();
            m_panelsManager->GetPanelAs<UI::Panels::AView>("Scene View").UnBind();
            uiManager->Render();
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            device->PollEvents();
            window->SwapBuffers();
            inputManager->SetDeltaTime(clock.GetDeltaTime());
            //inputManager->ClearEvents();
            core::Clock::Update();
            clock.Update();

        }
        //回收资源
        device.reset();
        ::core::Log::Shutdown();
        uiManager.reset();
        m_panelsManager.reset();
        inputManager.reset();
        window.reset();
    }
    return 0;
}

//
#include "Opengl/pch.h"
#include "Opengl/core/base.h"
#include "Opengl/core/clock.h"
#include "Opengl/core/sync.h"
#include "Opengl/asset/all.h"
#include "Opengl/component/all.h"
#include "Opengl/scene/renderer.h"
#include "Opengl/scene/ui.h"
#include "Opengl/util/ext.h"
#include "Opengl/util/math.h"
#include "Opengl/util/path.h"

using namespace core;
using namespace asset;
using namespace component;
using namespace utils;
namespace scene {

    static float skybox_exposure = 1.0f;
    static float skybox_lod = 0.0f;
    static float bounce_time = 0.0f;
    static int shadow_with = 1024;
    static bool  bounce_ball = false;
    static float PCF_SampleRadius = 1.0f;
    static float lightWidth=1.0f;  // 光源的宽度
    static float SMDiffuse=1.0f;   // 阴影的弥散程度
    static float  BIAS = 0.0005f;//偏移
    static int shadow_type = 0;
    void Scene07::PrecomputeIBL(const std::string&hdri) {
        Renderer::SeamlessCubemap(true);
        Renderer::DepthTest(false);
        Renderer::FaceCulling(true);

        auto irradiance_shader = CShader("res\\shaders\\irradiance_map.glsl");
        auto prefilter_shader = CShader("res\\shaders\\prefilter_envmap.glsl");
        auto envBRDF_shader = CShader("res\\shaders\\environment_BRDF.glsl");

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
    void Scene07::SetupMaterial(Material& pbr_mat, int mat_id) {
        pbr_mat.SetTexture(pbr_t::irradiance_map, irradiance_map);
        pbr_mat.SetTexture(pbr_t::prefiltered_map, prefiltered_map);
        pbr_mat.SetTexture(pbr_t::BRDF_LUT, BRDF_LUT);
        if (mat_id == 0) {  // floor
            pbr_mat.SetUniform(pbr_u::uv_scale, vec2(32.0f));
            pbr_mat.SetTexture(pbr_t::albedo, MakeAsset<Texture>("res\\texture\\wood_parquet_15-1K\\albedo.jpg"));
            pbr_mat.SetTexture(pbr_t::normal, MakeAsset<Texture>("res\\texture\\wood_parquet_15-1K\\normal.jpg"));
            pbr_mat.SetUniform(pbr_u::metalness, 0.0f);
            pbr_mat.SetUniform(pbr_u::roughness, 0.67f);
        }
        else if (mat_id == 1) {  // wall
            pbr_mat.SetUniform(pbr_u::albedo, vec4(1.0f, 0.6f, 1.0f, 1.0f));
            pbr_mat.SetUniform(pbr_u::roughness, 0.37f);
        }
        else if (mat_id == 2) {  // ball 0
            pbr_mat.SetUniform(pbr_u::albedo, vec4(0.0f, 0.63f, 0.0f, 1.0f));
            pbr_mat.SetUniform(pbr_u::roughness, 0.25f);
        }
        else if (mat_id == 3) {  // ball 1
            pbr_mat.SetUniform(pbr_u::albedo, vec4(0.83f, 0.83f, 0.32f, 1.0f));
            pbr_mat.SetUniform(pbr_u::metalness, 1.0f);
            pbr_mat.SetUniform(pbr_u::roughness, 0.74f);
        }


    
    }
    void Scene07::Init() {
        Renderer::SetScene(this);
        PrecomputeIBL("res\\texture\\HDRI\\Field-Path-Fence-Steinbacher-Street-4K.hdr");
        this->title = "Shadow";

        resource_manager.Add(00, MakeAsset<CShader>("res\\shaders\\bloom.glsl"));
        resource_manager.Add(01, MakeAsset<Shader>("res\\shaders\\infinite_grid.glsl"));
        resource_manager.Add(02, MakeAsset<Shader>("res\\shaders\\skybox.glsl"));
        resource_manager.Add(03, MakeAsset<Shader>("res\\shaders\\light.glsl"));
        resource_manager.Add(04, MakeAsset<Shader>("res\\shaders\\pbr07.glsl"));
        resource_manager.Add(05, MakeAsset<Shader>("res\\shaders\\post_process05.glsl"));
        resource_manager.Add(06, MakeAsset<Shader>("res\\shaders\\shadow07.glsl"));
        resource_manager.Add(07, MakeAsset<Shader>("res\\shaders\\shadow07vsm.glsl"));
        resource_manager.Add(12, MakeAsset<Material>(resource_manager.Get<Shader>(02)));
        resource_manager.Add(13, MakeAsset<Material>(resource_manager.Get<Shader>(03)));
        resource_manager.Add(14, MakeAsset<Material>(resource_manager.Get<Shader>(04)));
        resource_manager.Add(98, MakeAsset<Sampler>(FilterMode::Point));
        resource_manager.Add(99, MakeAsset<Sampler>(FilterMode::Bilinear));

        AddUBO(resource_manager.Get<Shader>(02)->ID());
        AddUBO(resource_manager.Get<Shader>(03)->ID());
        AddUBO(resource_manager.Get<Shader>(04)->ID());

        camera = CreateEntity("Camera", ETag::MainCamera);
        camera.GetComponent<Transform>().Translate(0.0f, 6.0f, 9.0f);
        camera.AddComponent<Camera>(View::Perspective);

        skybox = CreateEntity("Skybox", ETag::Skybox);
        skybox.AddComponent<Mesh>(Primitive::Cube);
        if (auto& mat = skybox.AddComponent<Material>(resource_manager.Get<Material>(12)); true) {
            mat.SetTexture(0, prefiltered_map);
            mat.BindUniform(0, &skybox_exposure);
            mat.BindUniform(1, &skybox_lod);
        }

        point_light = CreateEntity("Point Light");
        point_light.AddComponent<Mesh>(Primitive::Cube);
        point_light.GetComponent<Transform>().Translate(world::up * 6.0f);
        point_light.GetComponent<Transform>().Translate(world::forward * -4.0f);
        point_light.GetComponent<Transform>().Scale(0.05f);
        point_light.AddComponent<PointLight>(color::orange, 3.8f);
        point_light.GetComponent<PointLight>().SetAttenuation(0.03f, 0.015f);

        if (auto& mat = point_light.AddComponent<Material>(resource_manager.Get<Material>(13)); true) {
            auto& pl = point_light.GetComponent<PointLight>();
            mat.SetUniform(3, pl.color);
            mat.SetUniform(4, pl.intensity);
            mat.SetUniform(5, 2.0f);
        }

        spotlight = CreateEntity("Spotlight");
        spotlight.AddComponent<Mesh>(Primitive::Sphere);
        spotlight.GetComponent<Transform>().Translate(vec3(0.0f, 10.0f, -7.0f));
        spotlight.GetComponent<Transform>().Scale(0.1f);
        spotlight.AddComponent<Spotlight>(color::white, 13.8f);
        spotlight.GetComponent<Spotlight>().SetCutoff(20.0f, 10.0f, 45.0f);

        if (auto& mat = spotlight.AddComponent<Material>(resource_manager.Get<Material>(13)); true) {
            auto& sl = spotlight.GetComponent<Spotlight>();
            mat.SetUniform(3, sl.color);
            mat.SetUniform(4, sl.intensity);
            mat.SetUniform(5, 2.0f);
        }
        floor = CreateEntity("Floor");
        floor.AddComponent<Mesh>(Primitive::Plane);
        floor.GetComponent<Transform>().Translate(0.0f, -1.05f, 0.0f);
        floor.GetComponent<Transform>().Scale(20.0f);
        SetupMaterial(floor.AddComponent<Material>(resource_manager.Get<Material>(14)), 0);
        wall = CreateEntity("Wall");
        wall.AddComponent<Mesh>(Primitive::Cube);
        wall.GetComponent<Transform>().Translate(0.0f, 5.0f, -8.0f);
        wall.GetComponent<Transform>().Scale(12.0f, 6.0f, 0.25f);
        SetupMaterial(wall.AddComponent<Material>(resource_manager.Get<Material>(14)), 1);
        for (int i = 0; i < 3; i++) {
            ball[i] = CreateEntity("Sphere " + std::to_string(i));
            ball[i].GetComponent<Transform>().Translate(vec3(-i * 4, (i + 1) * 2, pow(-1, i) - 4));
            ball[i].AddComponent<Mesh>(Primitive::Sphere);
            SetupMaterial(ball[i].AddComponent<Material>(resource_manager.Get<Material>(14)), i + 2);
        }
        
        Renderer::MSAA(true);
        Renderer::DepthTest(true);
        Renderer::AlphaBlend(true);
        Renderer::FaceCulling(true);
  
    }

    void Scene07::OnSceneRender(float dt) {
        auto& main_camera = camera.GetComponent<Camera>();
        if (m_panelsManager->GetPanelAs<UI::Panels::AView>("Scene View").IsHovered())
            main_camera.Update();


        float& near_clip = main_camera.near_clip;
        float& far_clip = main_camera.far_clip;
        mat4 projection = glm::perspective(glm::radians(main_camera.fov), 1.0f, near_clip, far_clip);
        auto& pl_t = point_light.GetComponent<Transform>();
        pl_t.GetLocalTransform();
        mat4 pl_transform = projection *pl_t.GetLocalTransform();;


        if (auto& ubo = UBOs[0]; true) {
            ubo.SetUniform(0, val_ptr(main_camera.T->position));
            ubo.SetUniform(1, val_ptr(main_camera.T->forward));
            ubo.SetUniform(2, val_ptr(main_camera.GetViewMatrix()));
            ubo.SetUniform(3, val_ptr(main_camera.GetProjectionMatrix()));
        }
        if (auto& ubo = UBOs[1]; true) {
            auto& pl_1 = point_light.GetComponent<PointLight>();
            auto& pt_1 = point_light.GetComponent<Transform>();


            // for uniform arrays in std140, every element is padded to a vec4 (16 bytes)
            vec4 color[] = { vec4(pl_1.color, 0) };
            vec4 position[] = { vec4(pt_1.position, 0) };
            vec4 intensity[] = { vec4(pl_1.intensity) };
            vec4 linear[] = { vec4(pl_1.linear) };
            vec4 quadratic[] = { vec4(pl_1.quadratic) };
            vec4 range[] = { vec4(pl_1.range) };
            mat4 light_transform[] = { pl_transform};

            ubo.SetUniform(0, color);
            ubo.SetUniform(1, position);
            ubo.SetUniform(2, intensity);
            ubo.SetUniform(3, linear);
            ubo.SetUniform(4, quadratic);
            ubo.SetUniform(5, range);
            ubo.SetUniform(6, light_transform);
            ubo.SetUniform(7,&lightWidth);
            ubo.SetUniform(8, &SMDiffuse);
            ubo.SetUniform(9, &PCF_SampleRadius);
            ubo.SetUniform(10, &BIAS);
            ubo.SetUniform(11,&shadow_type);
        }

        if (auto& ubo = UBOs[2]; true) {
            auto& sl = spotlight.GetComponent<Spotlight>();
            auto& st = spotlight.GetComponent<Transform>();
            float inner_cos = sl.GetInnerCosine();
            float outer_cos = sl.GetOuterCosine();
            ubo.SetUniform(0, val_ptr(sl.color));
            ubo.SetUniform(1, val_ptr(st.position));
            ubo.SetUniform(2, val_ptr(st.up));
            ubo.SetUniform(3, val_ptr(sl.intensity));
            ubo.SetUniform(4, val_ptr(inner_cos));
            ubo.SetUniform(5, val_ptr(outer_cos));
            ubo.SetUniform(6, val_ptr(sl.range));
        }
        // update entities
         // simulate balls gravity using a cheap quadratic easing factor
        if (bounce_ball) {
        bounce_time += Clock::delta_time;
        for (int i = 0; i < 3; ++i) {
            float h = (i + 1) * 2.0f;                       // initial height of the ball
            float s = 2.0f - i * 0.4f;                      // falling and bouncing speed
            float t = math::Bounce(bounce_time * s, 1.0f);  // bounce between 0.0 and 1.0
            float y = math::Lerp(h, -0.05f, t * t);         // ease in, slow near 0 and fast near 1
            auto& T = ball[i].GetComponent<Transform>();
            T.SetPosition(vec3(T.position.x, y, T.position.z));
        }
        }

        FBO& framebuffer_0 = FBOs[0];
        FBO& framebuffer_1 = FBOs[1];
        FBO& framebuffer_2 = FBOs[2];
        FBO& framebuffer_3 = FBOs[3];
        FBO& framebuffer_4 = FBOs[4];
        FBO& framebuffer_5 = FBOs[5];
        // ------------------------------ shadow pass 1 ------------------------------


        framebuffer_0.Clear(-1);
        framebuffer_0.Bind();
        auto shadow_shader = resource_manager.Get<Shader>(06);
        shadow_shader->SetUniform(250, pl_transform);
        shadow_shader->SetUniform(5, false);
        Renderer::Submit(ball[0].id, ball[1].id, ball[2].id);
        Renderer::Submit(wall.id, floor.id);
        Renderer::Render(shadow_shader);
        framebuffer_0.Unbind();

        framebuffer_1.Clear();
        framebuffer_1.Bind();
       
        shadow_shader->SetUniform(250, pl_transform);
        shadow_shader->SetUniform(5, true);
        Renderer::Submit(ball[0].id, ball[1].id, ball[2].id);
        Renderer::Submit(wall.id, floor.id);
        Renderer::Render(shadow_shader);
        framebuffer_1.Unbind();

        framebuffer_1.GetColorTexture(0).Bind(14);
        framebuffer_5.Bind();
        auto shadowvsm_shader = resource_manager.Get<Shader>(07);
        shadowvsm_shader->Bind();shadowvsm_shader->SetUniform(5,true);
        Mesh::DrawQuad();
        shadowvsm_shader->Bind();shadowvsm_shader->SetUniform(5,false);
        Mesh::DrawQuad();
        framebuffer_5.Unbind();




        // ------------------------------ MRT render pass ------------------------------
        framebuffer_0.GetDepthTexture().Bind(15);
        framebuffer_5.GetColorTexture(0).Bind(13);
        framebuffer_2.Clear();
        framebuffer_2.Bind();

        Renderer::Submit(floor.id);
        Renderer::Submit(point_light.id);
        Renderer::Submit(skybox.id);
        Renderer::Submit(ball[0].id, ball[1].id, ball[2].id);

        Renderer::Submit(wall.id);

        Renderer::Submit(spotlight.id);
        Renderer::Render();


        framebuffer_2.Unbind();



        // ------------------------------ MSAA resolve pass ------------------------------

        framebuffer_3.Clear();
        FBO::CopyColor(framebuffer_2, 0, framebuffer_3, 0);
        FBO::CopyColor(framebuffer_2, 1, framebuffer_3, 1);

        // ------------------------------ apply Gaussian blur ------------------------------

        FBO::CopyColor(framebuffer_3, 1, framebuffer_4, 0);  // downsample the bloom target (nearest filtering)
        auto& ping = framebuffer_4.GetColorTexture(0);
        auto& pong = framebuffer_4.GetColorTexture(1);
        auto bloom_shader = resource_manager.Get<CShader>(00);

        bloom_shader->Bind();
        ping.BindILS(0, 0, GL_READ_WRITE);
        pong.BindILS(0, 1, GL_READ_WRITE);

        for (int i = 0; i < 6; ++i) {
            bloom_shader->SetUniform(0, i % 2 == 0);
            bloom_shader->Dispatch(ping.width / 32 + 1, ping.width / 18 + 1);
            bloom_shader->SyncWait(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
        }

    }

    void Scene07::OnImGuiRender(float dt) {   
        static float bias = 0.05;
        ImGui::Checkbox("Ball Bounce", &bounce_ball);
        ImGui::SliderFloat("PCF_SampleRadius",&PCF_SampleRadius,0,5);
        ImGui::SliderFloat("LightWidth", &lightWidth, 0, 5);
        ImGui::SliderFloat("SMDiffuse", &SMDiffuse, 0, 5);
        ImGui::SliderFloat("BIAS",&bias,0.05,0.5);
        BIAS = bias / 100.0;
        ImGui::RadioButton("shadow map",&shadow_type,0);
        ImGui::RadioButton("PCF", &shadow_type, 1);
        ImGui::RadioButton("PCSS", &shadow_type, 2);
        ImGui::RadioButton("VSM", &shadow_type, 3);
       
        ImGui::Text("this is a test");
        ImGui::Image((void*)FBOs[0].GetDepthTexture().ID(), ImVec2(500, 500), ImVec2(0.f, 1.f), ImVec2(1.f, 0.f));
        ImGui::Image((void*)FBOs[5].GetColorTexture(0).ID(), ImVec2(500, 500), ImVec2(0.f, 1.f), ImVec2(1.f, 0.f));
        if (ImGui::TreeNode("Entity")) {
            static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Selected |
                ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            static Entity e;
            for (int i = 0; i < directory_Entity.size(); i++) {
                ImGui::TreeNodeEx(directory_Entity[i].name.c_str(), base_flags);
                if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
                    e = directory_Entity[i];
                    CORE_INFO("{0} is selected", directory_Entity[i].name);
                    selected_entity = i;
                }
            }
            if (e.id != entt::null) {
                ImGui::RadioButton("T", &cur_operation, 0); ImGui::SameLine();
                ImGui::RadioButton("R", &cur_operation, 1); ImGui::SameLine();
                ImGui::RadioButton("S", &cur_operation, 2);
            }
            ImGui::TreePop();
        }
        else {
            selected_entity = -1;
        }
    }
    void Scene07::Resize(int w, int h)
    {
        camera.GetComponent<Camera>().aspect = 1.0f * w / h;
        Scene::Resize(w, h);
        FBOs.clear();

        AddFBO(shadow_with, shadow_with);
        AddFBO(shadow_with, shadow_with);
        AddFBO(w, h);
        AddFBO(w, h);
        AddFBO(w / 2, h / 2);
        AddFBO(shadow_with, shadow_with);

        FBOs[0].AddDepStTexture();
        FBOs[5].AddColorTexture(1);
        FBOs[1].AddDepStTexture();
        FBOs[1].AddColorTexture(1);
        FBOs[2].AddColorTexture(2, true);    // multisampled textures for MSAA
        FBOs[2].AddDepStRenderBuffer(true);  // multisampled RBO for MSAA
        FBOs[3].AddColorTexture(2);
        FBOs[4].AddColorTexture(2);
    }
    void Scene07::Present()
    {
        FBO& framebuffer_3 = FBOs[3];
        FBO& framebuffer_4 = FBOs[4];
        // ------------------------------ postprocessing pass ------------------------------

       // FBOs[0].GetDepthTexture().Bind(0);
        framebuffer_3.GetColorTexture(0).Bind(0);  // color texture
        framebuffer_4.GetColorTexture(0).Bind(1);  // bloom texture

        auto bilinear_sampler = resource_manager.Get<Sampler>(99);
        bilinear_sampler->Bind(1);  // upsample the bloom texture (bilinear filtering)

        auto postprocess_shader = resource_manager.Get<Shader>(05);
        postprocess_shader->Bind();
        postprocess_shader->SetUniform(0, 3);  // select tone-mapping operator

        Renderer::Clear();
        Mesh::DrawQuad();

        postprocess_shader->Unbind();
        bilinear_sampler->Unbind(1);

    }
}




