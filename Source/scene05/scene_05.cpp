#include"scene_05.h"

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
        scene::Scene* S = new scene::Scene05("PCSS Shadow and Animation");
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
                ImGuizmo::SetOrthographic(false);
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

#include "Opengl/pch.h"
#include "Opengl/core/base.h"
#include "Opengl/core/clock.h"
#include "Opengl/core/input.h"
#include "Opengl/core/window.h"
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
    static bool  show_grid = false;
    static float grid_cell_size = 2.0f;
    static vec4  thin_line_color = vec4(0.1f, 0.1f, 0.1f, 1.0f);
    static vec4  wide_line_color = vec4(0.2f, 0.2f, 0.2f, 1.0f);

    static float skybox_exposure = 1.0f;
    static float skybox_lod = 0.0f;
    static int   tab_id = 0;
    static bool  bounce_ball = false;
    static float bounce_time = 0.0f;
    static bool  enable_spotlight = false;
    static bool  enable_moonlight = false;
    static bool  enable_lantern = false;
    static bool  enable_shadow = false;
    static bool  animate_suzune = false;
    static float animate_speed = 1.0f;
    static float light_radius = 0.001f;

    constexpr uint shadow_width = 2048;
    constexpr uint shadow_height = 2048;
    void Scene05::Init() {
        Renderer::SetScene(this);
        PrecomputeIBL("res\\texture\\HDRI\\Field-Path-Fence-Steinbacher-Street-4K.hdr");
        this->title = "PCSS Shadow and Animation";

        resource_manager.Add(00, MakeAsset<CShader>("res\\shaders\\bloom.glsl"));
        resource_manager.Add(01, MakeAsset<Shader>("res\\shaders\\infinite_grid.glsl"));
        resource_manager.Add(02, MakeAsset<Shader>("res\\shaders\\skybox.glsl"));
        resource_manager.Add(03, MakeAsset<Shader>("res\\shaders\\light.glsl"));
        resource_manager.Add(04, MakeAsset<Shader>( "res\\shaders\\pbr05.glsl"));
        resource_manager.Add(05, MakeAsset<Shader>( "res\\shaders\\post_process05.glsl"));
        resource_manager.Add(06, MakeAsset<Shader>("res\\shaders\\shadow05.glsl"));
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

        moonlight = CreateEntity("Moonlight");
        moonlight.GetComponent<Transform>().Rotate(-45.0f, 0.0f, 0.0f, Space::World);
        moonlight.AddComponent<DirectionLight>(vec3(0.0f, 0.43f, 1.0f), 0.5f);

        // moonlight is static so we only need to set the uniform buffer once in `Init()`
        if (auto& ubo = UBOs[3]; true) {
            auto& dl = moonlight.GetComponent<DirectionLight>();
            auto& dt = moonlight.GetComponent<Transform>();

            vec4 R = vec4(dt.right, 0.0f);
            vec4 F = vec4(dt.forward, 0.0f);
            vec4 U = vec4(dt.up, 0.0f);
            vec4 directions[] = { -F, -U, R, -R, vec4(world::backward, 0.0f) };

            ubo.SetUniform(0, val_ptr(dl.color));
            ubo.SetUniform(1, directions);
            ubo.SetUniform(2, val_ptr(dl.intensity));
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
            if (i == 0) {
                auto& model=ball[i].AddComponent<Model>("res\\PathTrace\\Scenes\\Box\\alien-20.obj", Quality::Auto, true);
                SetupMaterial(model.SetMaterial("DefaultMaterial", resource_manager.Get<Material>(14)), i + 2);
            }
            else
            ball[i].AddComponent<Mesh>(Primitive::Sphere);
            SetupMaterial(ball[i].AddComponent<Material>(resource_manager.Get<Material>(14)), i + 2);
        }
        suzune = CreateEntity("Nekomimi Suzune");
        suzune.GetComponent<Transform>().Translate(vec3(0.0f, -0.9f, 0.0f));
        suzune.GetComponent<Transform>().Scale(0.05f);
 
        if (std::string model_path =  "res\\model\\suzune\\suzune.fbx"; true) {
            auto& model = suzune.AddComponent<Model>(model_path, Quality::Auto, true);
           
            auto& animator = suzune.AddComponent<Animator>();
            animator.Update( Clock::delta_time);

            SetupMaterial(model.SetMaterial("mat_Suzune_Hair.001", resource_manager.Get<Material>(14)), 50);
            SetupMaterial(model.SetMaterial("mat_Suzune_Body.001", resource_manager.Get<Material>(14)), 51);
            SetupMaterial(model.SetMaterial("mat_Suzune_Cloth.001", resource_manager.Get<Material>(14)), 52);
            SetupMaterial(model.SetMaterial("mat_Suzune_Head.001", resource_manager.Get<Material>(14)), 53);
            SetupMaterial(model.SetMaterial("mat_Suzune_EyeL.001", resource_manager.Get<Material>(14)), 54);
            SetupMaterial(model.SetMaterial("mat_Suzune_EyeR.001", resource_manager.Get<Material>(14)), 55);
        }
        mingyue = CreateEntity("Mingyue");
        mingyue.GetComponent<Transform>().Translate(vec3(2.0f, -0.9f, -3.0f));
        mingyue.GetComponent<Transform>().Scale(5.0f);
        //BarberShopChair_01_4k.gltf
        //"res\\mingyue\\Hero_Mingyue\\Hero_Mingyue.fbx"
        if (std::string model_path = "res\\mingyue\\Hero_Mingyue\\Hero_Mingyue.fbx";true) {
            auto& model = mingyue.AddComponent<Model>(model_path, Quality::Auto, true);

            auto& animator = mingyue.AddComponent<Animator>();
            animator.Update(Clock::delta_time);
            SetupMaterial(model.SetMaterial("Hero_Mingyue_Hair", resource_manager.Get<Material>(14)), 56);
            SetupMaterial(model.SetMaterial("Hero_Mingyue_Body_sha", resource_manager.Get<Material>(14)), 57);
            SetupMaterial(model.SetMaterial("Hero_Mingyue_Face", resource_manager.Get<Material>(14)), 58);
            SetupMaterial(model.SetMaterial("Hero_Mingyue_Body", resource_manager.Get<Material>(14)), 59);
        }
        korean_fire = CreateEntity("korean_fire");
        if (std::string model_path = "res\\model\\korean_fire_extinguisher_01_4k.gltf"; true) {
            auto& model = korean_fire.AddComponent<Model>(model_path, Quality::Auto, true);
            SetupMaterial(model.SetMaterial("korean_fire_extinguisher_01_body", resource_manager.Get<Material>(14)), 60);
            SetupMaterial(model.SetMaterial("korean_fire_extinguisher_01_glass", resource_manager.Get<Material>(14)), 60);
            SetupMaterial(model.SetMaterial("korean_fire_extinguisher_01_paper", resource_manager.Get<Material>(14)), 60);


        }


        Renderer::MSAA(true);
        Renderer::DepthTest(true);
        Renderer::AlphaBlend(true);
        Renderer::FaceCulling(true);

    }
    void Scene05::Resize(int w, int h) {
        camera.GetComponent<Camera>().aspect = 1.0f * w / h;
        Scene::Resize(w, h);
        FBOs.clear();

        AddFBO(shadow_width, shadow_height);
        AddFBO(shadow_width, shadow_height);
        AddFBO(w, h);
        AddFBO(w, h);
        AddFBO(w / 2, h / 2);

        FBOs[0].AddDepthCubemap();
        FBOs[1].AddDepthCubemap();
        FBOs[2].AddColorTexture(2, true);    // multisampled textures for MSAA
        FBOs[2].AddDepStRenderBuffer(true);  // multisampled RBO for MSAA
        FBOs[3].AddColorTexture(2);
        FBOs[4].AddColorTexture(2);



    }
    void Scene05::Present() {
        FBO& framebuffer_3 = FBOs[3];
        FBO& framebuffer_4 = FBOs[4];
        // ------------------------------ postprocessing pass ------------------------------

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
    void Scene05::OnSceneRender(float dt) {
        auto& main_camera = camera.GetComponent<Camera>();
        if (m_panelsManager->GetPanelAs<UI::Panels::AView>("Scene View").IsHovered())
            main_camera.Update();

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
            vec4 position[] = { vec4(pt_1.position, 0)};
            vec4 intensity[] = { vec4(pl_1.intensity) };
            vec4 linear[] = { vec4(pl_1.linear) };
            vec4 quadratic[] = { vec4(pl_1.quadratic) };
            vec4 range[] = { vec4(pl_1.range) };

            ubo.SetUniform(0, color);
            ubo.SetUniform(1, position);
            ubo.SetUniform(2, intensity);
            ubo.SetUniform(3, linear);
            ubo.SetUniform(4, quadratic);
            ubo.SetUniform(5, range);
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
        bounce_time += Clock::delta_time;
        if(bounce_ball)
        for (int i = 0; i < 3; ++i) {
            float h = (i + 1) * 2.0f;                       // initial height of the ball
            float s = 2.0f - i * 0.4f;                      // falling and bouncing speed
            float t = math::Bounce(bounce_time * s, 1.0f);  // bounce between 0.0 and 1.0
            float y = math::Lerp(h, -0.05f, t * t);         // ease in, slow near 0 and fast near 1
            auto& T = ball[i].GetComponent<Transform>();
            T.SetPosition(vec3(T.position.x, y, T.position.z));
        }
        auto& animator = suzune.GetComponent<Animator>();
        animator.Update( Clock::delta_time * animate_speed);
        mingyue.GetComponent<Animator>().Update(Clock::delta_time * animate_speed);

        FBO& framebuffer_0 = FBOs[0];
        FBO& framebuffer_1 = FBOs[1];
        FBO& framebuffer_2 = FBOs[2];
        FBO& framebuffer_3 = FBOs[3];
        FBO& framebuffer_4 = FBOs[4];
        
        
        // ------------------------------ shadow pass 1 ------------------------------

        Renderer::SetViewport(shadow_width, shadow_height);
        Renderer::SetShadowPass(1);
        framebuffer_0.Clear(-1);
        framebuffer_0.Bind();
        auto shadow_shader = resource_manager.Get<Shader>(06);

        auto& bone_transforms = suzune.GetComponent<Animator>().bone_transforms;
        for (size_t i = 0; i < bone_transforms.size(); ++i) {
                shadow_shader->SetUniform(100 + i, bone_transforms[i]);
        }
        

        float& near_clip = main_camera.near_clip;
        float& far_clip = main_camera.far_clip;
        mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, near_clip, far_clip);

        auto& pl_t = point_light.GetComponent<Transform>();
        std::vector<mat4> pl_transform = {
            projection * pl_t.GetLocalTransform(world::right,    world::down),
            projection * pl_t.GetLocalTransform(world::left,     world::down),
            projection * pl_t.GetLocalTransform(world::up,       world::backward),
            projection * pl_t.GetLocalTransform(world::down,     world::forward),
            projection * pl_t.GetLocalTransform(world::backward, world::down),
            projection * pl_t.GetLocalTransform(world::forward,  world::down)
        };

        shadow_shader->SetUniformArray(250, 6, pl_transform);


        shadow_shader->SetUniform(1008,true);
        Renderer::Submit(suzune.id); 
        Renderer::Render(shadow_shader);

        auto& bonetransforms = mingyue.GetComponent<Animator>().bone_transforms;
        for (size_t i = 0; i < bonetransforms.size(); ++i) {
            shadow_shader->SetUniform(100 + i, bonetransforms[i]);
        }
        Renderer::Submit(mingyue.id);
        Renderer::Render(shadow_shader);

        shadow_shader->SetUniform(1008, false);
        Renderer::Submit(ball[0].id, ball[1].id, ball[2].id);
        Renderer::Submit(wall.id,floor.id);
        Renderer::Submit(floor.id);
        Renderer::Submit(korean_fire.id);

        Renderer::Render(shadow_shader);
        Renderer::SetViewport(Window::width, Window::height);
        Renderer::SetShadowPass(0);
        // ------------------------------ MRT render pass ------------------------------
        framebuffer_0.GetDepthTexture().Bind(15);
        framebuffer_2.Clear();
        framebuffer_2.Bind();   
        Renderer::Submit(suzune.id);
        Renderer::Submit(mingyue.id);

        Renderer::Submit(korean_fire.id);
        Renderer::Submit(floor.id);
        Renderer::Submit(point_light.id);
        Renderer::Submit(skybox.id);
        Renderer::Submit(ball[0].id, ball[1].id, ball[2].id);

        Renderer::Submit(wall.id);
  
        Renderer::Submit(spotlight.id);

        Renderer::Render();
        if (show_grid) {
            auto grid_shader = resource_manager.Get<Shader>(01);
            grid_shader->Bind();
            grid_shader->SetUniform(0, grid_cell_size);
            grid_shader->SetUniform(1, thin_line_color);
            grid_shader->SetUniform(2, wide_line_color);
            Mesh::DrawGrid();
        }

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
            bloom_shader->Dispatch(ping.width / 32+1, ping.width / 18+1);
            bloom_shader->SyncWait(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
        }
    }
    void Scene05::OnImGuiRender(float dt) {
        using namespace ImGui;
        const ImVec4 tab_color_off = ImVec4(0.0f, 0.3f, 0.6f, 1.0f);
        const ImVec4 tab_color_on = ImVec4(0.0f, 0.4f, 0.8f, 1.0f);
        const ImGuiColorEditFlags color_flags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha;
        static bool show_gizmo_pl = false;
        static bool show_gizmo_sl = false;
        static bool show_gizmo_lt = false;
        static vec3 lantern_color = color::white;
        static float v = 1.0;

        if (true) {
            Indent(5.0f);
            PushItemWidth(130.0f);
            SliderFloat("Skybox Exposure", &skybox_exposure, 0.5f, 4.0f);
            SliderFloat("Skybox LOD", &skybox_lod, 0.0f, 7.0f);
            PopItemWidth();
            Separator();

            BeginTabBar("InspectorTab", ImGuiTabBarFlags_None);

            if (BeginTabItem("Bouncing Ball")) {
                tab_id = 0;
                enable_spotlight = enable_moonlight = enable_lantern = false;
                show_gizmo_sl = show_gizmo_lt = false;
                PushItemWidth(130.0f);
                Checkbox("Enable Shadow", &enable_shadow);
                Checkbox("Show Gizmo PL", &show_gizmo_pl);
                Checkbox("Ball Bounce", &bounce_ball);
                SliderFloat("Light Radius", &light_radius, 0.001f, 0.1f);
                PopItemWidth();
                EndTabItem();
            }

            if (BeginTabItem("Nekomimi")) {
                tab_id = 1;
                enable_spotlight = true;
                enable_lantern = show_gizmo_lt = false;
                PushItemWidth(130.0f);
                Checkbox("Enable Shadow", &enable_shadow);
                Checkbox("Enable Moonlight", &enable_moonlight);
                Checkbox("Show Gizmo PL", &show_gizmo_pl);
                Checkbox("Show Gizmo SL", &show_gizmo_sl);
                if (show_gizmo_pl && show_gizmo_sl) { show_gizmo_pl = false; }
                Checkbox("Play Animation", &animate_suzune);
                SliderFloat("Animation Speed", &animate_speed, 0.1f, 3.0f);
                if (Button("Go next")) {
                    auto& animator = mingyue.GetComponent<Animator>();
                    animator.Gonext();
                }
                SliderFloat("Light Radius", &light_radius, 0.001f, 0.1f);
                PopItemWidth();
                EndTabItem();
            }
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
            PushStyleColor(ImGuiCol_Tab, tab_color_off);
            PushStyleColor(ImGuiCol_TabHovered, tab_color_on);
            PushStyleColor(ImGuiCol_TabActive, tab_color_on);
            if (BeginTabItem("Grid")) {
                PushItemWidth(130.0f);
                Checkbox("Show Infinite Grid", &show_grid);
                SliderFloat("Grid Cell Size", &grid_cell_size, 0.25f, 8.0f);
                PopItemWidth();
                ColorEdit4("Line Color Minor", val_ptr(thin_line_color), color_flags);
                ColorEdit4("Line Color Main", val_ptr(wide_line_color), color_flags);
                EndTabItem();
            }
            PopStyleColor(3);
            EndTabBar();
            Unindent(5.0f);
        }
    }
    void Scene05::PrecomputeIBL(const std::string& hdri)
    {
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
    void Scene05::SetupMaterial(Material& pbr_mat, int mat_id)
    {
        pbr_mat.SetTexture(pbr_t::irradiance_map, irradiance_map);
        pbr_mat.SetTexture(pbr_t::prefiltered_map, prefiltered_map);
        pbr_mat.SetTexture(pbr_t::BRDF_LUT, BRDF_LUT);

        pbr_mat.BindUniform(0, &skybox_exposure);
        pbr_mat.BindUniform(1, &enable_spotlight);
        pbr_mat.BindUniform(2, &enable_moonlight);
        pbr_mat.BindUniform(3, &enable_lantern);
        pbr_mat.BindUniform(4, &enable_shadow);
        pbr_mat.BindUniform(5, &light_radius);


        if (mat_id == 0) {  // floor
            pbr_mat.SetUniform(pbr_u::uv_scale, vec2(32.0f));
            pbr_mat.SetTexture(pbr_t::albedo, MakeAsset<Texture>( "res\\texture\\wood_parquet_15-1K\\albedo.jpg"));
            pbr_mat.SetTexture(pbr_t::normal, MakeAsset<Texture>( "res\\texture\\wood_parquet_15-1K\\normal.jpg"));
            pbr_mat.SetUniform(pbr_u::metalness, 0.0f);
            pbr_mat.SetUniform(pbr_u::roughness, 0.37f);
        }
        else if (mat_id == 1) {  // wall
            pbr_mat.SetUniform(pbr_u::albedo, vec4(1.0f, 0.6f, 1.0f, 1.0f));
            pbr_mat.SetUniform(pbr_u::roughness, 0.37f);
        }
        else if (mat_id == 2) {  // ball 0
            pbr_mat.SetUniform(pbr_u::albedo, vec4(1.0f, 0.0f, 0.0f, 1.0f));
            pbr_mat.SetUniform(pbr_u::roughness, 0.45f);
        }
        else if (mat_id == 3) {  // ball 1
            pbr_mat.SetUniform(pbr_u::albedo, vec4(0.83f, 0.83f, 0.32f, 1.0f));
            pbr_mat.SetUniform(pbr_u::metalness, 1.0f);
            pbr_mat.SetUniform(pbr_u::roughness, 0.74f);
        }
        else if (mat_id == 4) {  // ball 2
            pbr_mat.SetUniform(pbr_u::albedo, vec4(color::purple, 0.5f));
            pbr_mat.SetUniform(pbr_u::roughness, 0.3f);
        }
        else if (mat_id == 50) {  // hair (Nekomimi Suzune)
            //pbr_mat.SetTexture(pbr_t::albedo, MakeAsset<Texture>( "res\\model\\suzune\\Hair.png"));
            pbr_mat.SetUniform(pbr_u::roughness, 0.8f);
            pbr_mat.SetUniform(pbr_u::specular, 0.7f);
        }
        else if (mat_id == 51) {  // body (Nekomimi Suzune)
            //pbr_mat.SetTexture(pbr_t::albedo, MakeAsset<Texture>( "res\\model\\suzune\\Cloth.png"));
        }
        else if (mat_id == 52) {  // cloth (Nekomimi Suzune)
          //  pbr_mat.SetTexture(pbr_t::albedo, MakeAsset<Texture>( "res\\model\\suzune\\Body.png"));
        }
        else if (mat_id == 53) {  // head (Nekomimi Suzune)
           // pbr_mat.SetTexture(pbr_t::albedo, MakeAsset<Texture>( "res\\model\\suzune\\Head.png"));
        }
        else if (mat_id == 54) {  // L eye (Nekomimi Suzune)
           // pbr_mat.SetTexture(pbr_t::albedo, MakeAsset<Texture>( "res\\model\\suzune\\Head.png"));
            pbr_mat.SetUniform(pbr_u::roughness, 0.045f);
            pbr_mat.SetUniform(pbr_u::specular, 0.35f);
        }
        else if (mat_id == 55) {  // R eye (Nekomimi Suzune)
            //pbr_mat.SetTexture(pbr_t::albedo, MakeAsset<Texture>( "res\\model\\suzune\\Head2.png"));
            pbr_mat.SetUniform(pbr_u::roughness, 0.045f);
            pbr_mat.SetUniform(pbr_u::specular, 0.35f);
        }


        if (mat_id >= 50 && mat_id <= 55) {  // Nekomimi Suzune
            pbr_mat.SetUniform(1008,true);
            auto& bone_transforms = suzune.GetComponent<Animator>().bone_transforms;
            pbr_mat.SetUniformArray(100U, bone_transforms.size(), &bone_transforms);
           
        }
        if (mat_id > 55 && mat_id < 60) {//mingyue
            auto& bone_transforms = mingyue.GetComponent<Animator>().bone_transforms;
            pbr_mat.SetUniformArray(100U, bone_transforms.size(), &bone_transforms);
            pbr_mat.SetUniform(1008, true);
        }


    }
}