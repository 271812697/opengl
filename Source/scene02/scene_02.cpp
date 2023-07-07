#include "scene_02.h"

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
        windowSettings.title = "Environment Lighting (IBL)";
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
        scene::Scene* S = new scene::Scene02("Environment Lighting (IBL)");
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
//hash string
using ID = int;
inline ID StringToHash(std::string_view str) {
    static std::hash<std::string_view> h;
    return  h(str);
}

namespace scene {
    static float skybox_exposure = 1.0f;
    static float skybox_lod = 0.0f;

    static int   entity_id = 0;  // 0: sphere, 1: torus, 2: cube, 3: motorbike
    static bool  motor_wireframe = false;
    static float tank_roughness = 0.72f;

    static vec4  sphere_color[7]{ vec4(color::black, 1.0f) };
    static float sphere_metalness[7]{ 0.05f, 0.15f, 0.3f, 0.45f, 0.6f, 0.75f, 0.9f };
    static float sphere_roughness[7]{ 0.05f, 0.15f, 0.3f, 0.45f, 0.6f, 0.75f, 0.9f };
    static float sphere_ao = 0.5f;

    static float cube_metalness = 0.5f;
    static float cube_roughness = 0.5f;
    static int   cube_rotation = -1;  // -1: no rotation, 0: up, 1: left, 2: right, 3: down
    static int   rotation_mode = 1;
    static bool  reset_cube = false;

    static vec4  torus_color = vec4(color::white, 1.0f);
    static float torus_metalness = 0.5f;
    static float torus_roughness = 0.5f;
    static float torus_ao = 0.5f;
    static bool  rotate_torus = false;

    void Scene02::Init() {
        Renderer::SetScene(this);
        this->title = "Environment Lighting (IBL)";
        PrecomputeIBL( "res\\texture\\HDRI\\Field-Path-Fence-Steinbacher-Street-4K.hdr");

        resource_manager.Add(StringToHash("SphereMesh"), MakeAsset<Mesh>(Primitive::Sphere));
        resource_manager.Add(StringToHash("CubeMesh"), MakeAsset<Mesh>(Primitive::Cube));
        resource_manager.Add(StringToHash("bloom.glsl"), MakeAsset<CShader>("res\\shaders\\bloom.glsl"));
        resource_manager.Add(StringToHash("infinite_grid.glsl"), MakeAsset<Shader>( "res\\shaders\\infinite_grid.glsl"));
        resource_manager.Add(StringToHash("skybox.glsl"), MakeAsset<Shader>( "res\\shaders\\skybox.glsl"));
        resource_manager.Add(StringToHash("light.glsl"), MakeAsset<Shader>( "res\\shaders\\light.glsl"));
        resource_manager.Add(StringToHash("pbr.glsl"), MakeAsset<Shader>("res\\shaders\\pbr02.glsl"));
        resource_manager.Add(StringToHash("post_process.glsl"), MakeAsset<Shader>("res\\shaders\\post_process02.glsl"));
        resource_manager.Add(StringToHash("SkyboxMaterial"), MakeAsset<Material>(resource_manager.Get<Shader>(StringToHash("skybox.glsl"))));
        resource_manager.Add(StringToHash("lightMaterial"), MakeAsset<Material>(resource_manager.Get<Shader>(StringToHash("light.glsl"))));
        resource_manager.Add(StringToHash("PbrMaterial"), MakeAsset<Material>(resource_manager.Get<Shader>(StringToHash("pbr.glsl"))));
        resource_manager.Add(StringToHash("PointSampler"), MakeAsset<Sampler>(FilterMode::Point));
        resource_manager.Add(StringToHash("BilinearSampler"), MakeAsset<Sampler>(FilterMode::Bilinear));

        AddUBO(resource_manager.Get<Shader>(StringToHash("skybox.glsl"))->ID());
        AddUBO(resource_manager.Get<Shader>(StringToHash("light.glsl"))->ID());
        AddUBO(resource_manager.Get<Shader>(StringToHash("pbr.glsl"))->ID());


        camera = CreateEntity("Camera", ETag::MainCamera);
        camera.GetComponent<Transform>().Translate(0.0f, 6.0f, 9.0f);
        camera.AddComponent<Camera>(View::Perspective);

        skybox = CreateEntity("Skybox", ETag::Skybox);
        skybox.AddComponent<Mesh>(Primitive::Cube);
        if (auto& mat = skybox.AddComponent<Material>(resource_manager.Get<Material>(StringToHash("SkyboxMaterial"))); true) {
            mat.SetTexture(0, prefiltered_map);
            mat.BindUniform(0, &skybox_exposure);
            mat.BindUniform(1, &skybox_lod);
        }

        // create 10 spheres (3 w/ textures + 7 w/o textures)
        const float sphere_posx[] = { 0.0f, -1.5f, 1.5f, -3.0f, 0.0f, 3.0f, -4.5f, -1.5f, 1.5f, 4.5f };
        const float sphere_posy[] = { 10.5f, 7.5f, 7.5f, 4.5f, 4.5f, 4.5f, 1.5f, 1.5f, 1.5f, 1.5f };
        auto sphere_mesh = resource_manager.Get<Mesh>(StringToHash("SphereMesh"));

        for (int i = 0; i < 10; i++) {
            sphere[i] = CreateEntity("Sphere " + std::to_string(i));
            sphere[i].GetComponent<Transform>().Translate(sphere_posx[i] * world::left);
            sphere[i].GetComponent<Transform>().Translate(sphere_posy[i] * world::up);
            sphere[i].AddComponent<Mesh>(sphere_mesh);

            auto& material = sphere[i].AddComponent<Material>(resource_manager.Get<Material>(StringToHash("PbrMaterial")));
            SetupMaterial(material, i);
        }

        // create 3 cubes (2 translation + 1 rotation)
        for (int i = 0; i < 3; i++) {
            cube[i] = CreateEntity("Cube " + std::to_string(i));
            cube[i].AddComponent<Mesh>(resource_manager.Get<Mesh>(StringToHash("CubeMesh")));
            cube[i].GetComponent<Transform>().Translate(world::left * (i - 1) * 6);
            cube[i].GetComponent<Transform>().Translate(world::up * 5.0f);

            auto& material = cube[i].AddComponent<Material>(resource_manager.Get<Material>(StringToHash("PbrMaterial")));
            SetupMaterial(material, i + 10);
        }

        point_light = CreateEntity("Point Light");
        point_light.AddComponent<Mesh>(sphere_mesh);
        point_light.GetComponent<Transform>().Translate(world::up * 6.0f);
        point_light.GetComponent<Transform>().Translate(world::backward * 4.0f);
        point_light.GetComponent<Transform>().Scale(0.1f);
        point_light.AddComponent<PointLight>(color::orange, 1.8f);
        point_light.GetComponent<PointLight>().SetAttenuation(0.09f, 0.032f);

        if (auto& mat = point_light.AddComponent<Material>(resource_manager.Get<Material>(StringToHash("lightMaterial"))); true) {
            auto& pl = point_light.GetComponent<PointLight>();
            mat.SetUniform(3, pl.color);
            mat.SetUniform(4, pl.intensity);
            mat.SetUniform(5, 2.0f);
        }

        torus = CreateEntity("Torus");
        torus.AddComponent<Mesh>(Primitive::Torus);
        torus.GetComponent<Transform>().Translate(world::up * 5.0f);
        SetupMaterial(torus.AddComponent<Material>(resource_manager.Get<Material>(StringToHash("PbrMaterial"))), 20);

        motorbike = CreateEntity("Motorbike");
        motorbike.GetComponent<Transform>().Rotate(world::up, -90.0f, Space::Local);
        motorbike.GetComponent<Transform>().Scale(0.25f);
        motorbike.GetComponent<Transform>().Translate(vec3(10.0f, 0.0f, 5.0f));

        if (std::string model_path =  "res\\model\\motorbike\\"; true) {
            auto& model = motorbike.AddComponent<Model>(model_path + "motor.fbx", Quality::Auto);

            SetupMaterial(model.SetMaterial("24 - Default", resource_manager.Get<Material>(StringToHash("PbrMaterial"))), 30);
            SetupMaterial(model.SetMaterial("15 - Default", resource_manager.Get<Material>(StringToHash("PbrMaterial"))), 31);
            SetupMaterial(model.SetMaterial("18 - Default", resource_manager.Get<Material>(StringToHash("PbrMaterial"))), 32);
            SetupMaterial(model.SetMaterial("21 - Default", resource_manager.Get<Material>(StringToHash("PbrMaterial"))), 33);
            SetupMaterial(model.SetMaterial("23 - Default", resource_manager.Get<Material>(StringToHash("PbrMaterial"))), 34);
            SetupMaterial(model.SetMaterial("20 - Default", resource_manager.Get<Material>(StringToHash("PbrMaterial"))), 35);
            SetupMaterial(model.SetMaterial("17 - Default", resource_manager.Get<Material>(StringToHash("PbrMaterial"))), 36);
            SetupMaterial(model.SetMaterial("22 - Default", resource_manager.Get<Material>(StringToHash("PbrMaterial"))), 37);
            SetupMaterial(model.SetMaterial("Material #308", resource_manager.Get<Material>(StringToHash("PbrMaterial"))), 38);
            SetupMaterial(model.SetMaterial("Material #706", resource_manager.Get<Material>(StringToHash("PbrMaterial"))), 39);
        }

        Renderer::MSAA(true);
        Renderer::DepthTest(true);
        Renderer::AlphaBlend(true);
        Renderer::FaceCulling(true);
    
    }

    void Scene02::OnSceneRender(float dt ) {
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
            auto& pl = point_light.GetComponent<PointLight>();
            auto& pt = point_light.GetComponent<Transform>();
            ubo.SetUniform(0, val_ptr(pl.color));
            ubo.SetUniform(1, val_ptr(pt.position));
            ubo.SetUniform(2, val_ptr(pl.intensity));
            ubo.SetUniform(3, val_ptr(pl.linear));
            ubo.SetUniform(4, val_ptr(pl.quadratic));
            ubo.SetUniform(5, val_ptr(pl.range));
        }

        FBO& framebuffer_0 = FBOs[0];
        FBO& framebuffer_1 = FBOs[1];
        FBO& framebuffer_2 = FBOs[2];

        // ------------------------------ MRT render pass ------------------------------

        framebuffer_0.Clear();
        framebuffer_0.Bind();

        /**/ if (entity_id == 0) { RenderSphere(); }
        else if (entity_id == 1) { RenderTorus(); }
        else if (entity_id == 2) { RenderCubes(); }
        else if (entity_id == 3) { RenderMotor(); }

        Renderer::Submit(point_light.id);
        Renderer::Submit(skybox.id);
        Renderer::Render();

        framebuffer_0.Unbind();

        // ------------------------------ MSAA resolve pass ------------------------------

        framebuffer_1.Clear();
        FBO::CopyColor(framebuffer_0, 0, framebuffer_1, 0);
        FBO::CopyColor(framebuffer_0, 1, framebuffer_1, 1);

        // ------------------------------ apply Gaussian blur ------------------------------

        FBO::CopyColor(framebuffer_1, 1, framebuffer_2, 0);  // downsample the bloom target (nearest filtering)
        auto& ping = framebuffer_2.GetColorTexture(0);
        auto& pong = framebuffer_2.GetColorTexture(1);
        auto bloom_shader = resource_manager.Get<CShader>(StringToHash("bloom.glsl"));

        bloom_shader->Bind();
        ping.BindILS(0, 0, GL_READ_WRITE);
        pong.BindILS(0, 1, GL_READ_WRITE);

        for (int i = 0; i < 6; ++i) {
            bloom_shader->SetUniform(0, i % 2 == 0);
            bloom_shader->Dispatch(ping.width / 32, ping.height/ 18);
            bloom_shader->SyncWait(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
        }

    }

    void Scene02::OnImGuiRender(float dt ) {
        using namespace ImGui;
        const ImGuiColorEditFlags color_flags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha;
        static bool edit_sphere_metalness = false;
        static bool edit_sphere_roughness = false;
        static int z_mode = -1;  // cube gizmo mode

        // 3 x 3 cube rotation panel
        const bool cell_enabled[9] = { false, true, false, true, false, true, false, true, false };
        const char* cell_label[4] = { ICON_FK_LONG_ARROW_UP, ICON_FK_LONG_ARROW_LEFT, ICON_FK_LONG_ARROW_RIGHT, ICON_FK_LONG_ARROW_DOWN };
        const ImVec2 cell_size = ImVec2(40, 40);

        if (true) {
            Indent(5.0f);
            PushItemWidth(130.0f);
            SliderFloat("Skybox Exposure", &skybox_exposure, 0.5f, 2.0f);
            SliderFloat("Skybox LOD", &skybox_lod, 0.0f, 7.0f);
            PopItemWidth();
            Separator();

            Text("Entity to Render");
            Separator();
            RadioButton("Static Sphere", &entity_id, 0); SameLine(164);
            RadioButton("Color Torus", &entity_id, 1);
            RadioButton("Spinning Cube", &entity_id, 2); SameLine(164);
            RadioButton("MotorCycle", &entity_id, 3);
            Separator();

            if (CollapsingHeader("Static Sphere", ImGuiTreeNodeFlags_None)) {
                PushItemWidth(130.0f);
                SliderFloat("Ambient Occlusion##1", &sphere_ao, 0.05f, 0.5f);
                PopItemWidth();

                if (Checkbox("Edit Metalness", &edit_sphere_metalness); edit_sphere_metalness) {
                    PushID("Metalness Sliders");
                    for (int i = 0; i < 7; i++) {
                        float hue = i / 7.0f;
                        PushID(i);
                        PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor::HSV(hue, 0.5f, 0.5f));
                        PushStyleColor(ImGuiCol_FrameBgHovered, (ImVec4)ImColor::HSV(hue, 0.6f, 0.5f));
                        PushStyleColor(ImGuiCol_FrameBgActive, (ImVec4)ImColor::HSV(hue, 0.7f, 0.5f));
                        PushStyleColor(ImGuiCol_SliderGrab, (ImVec4)ImColor::HSV(hue, 0.9f, 0.9f));

                        VSliderFloat("##m", ImVec2(20, 160), &sphere_metalness[i], 0.0f, 1.0f, "");
                        if (IsItemActive() || IsItemHovered()) {
                            SetTooltip("%.3f", sphere_metalness[i]);
                        }

                        PopStyleColor(4);
                        PopID();
                        SameLine();
                    }
                    PopID();
                    NewLine();
                }

                if (Checkbox("Edit Roughness", &edit_sphere_roughness); edit_sphere_roughness) {
                    PushID("Roughness Sliders");
                    for (int i = 0; i < 7; i++) {
                        float hue = i / 7.0f;
                        PushID(i);
                        PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor::HSV(hue, 0.5f, 0.5f));
                        PushStyleColor(ImGuiCol_FrameBgHovered, (ImVec4)ImColor::HSV(hue, 0.6f, 0.5f));
                        PushStyleColor(ImGuiCol_FrameBgActive, (ImVec4)ImColor::HSV(hue, 0.7f, 0.5f));
                        PushStyleColor(ImGuiCol_SliderGrab, (ImVec4)ImColor::HSV(hue, 0.9f, 0.9f));

                        VSliderFloat("##r", ImVec2(20, 160), &sphere_roughness[i], 0.01f, 1.0f, "");
                        if (IsItemActive() || IsItemHovered()) {
                            SetTooltip("%.3f", sphere_roughness[i]);
                        }

                        PopStyleColor(4);
                        PopID();
                        SameLine();
                    }
                    PopID();
                    NewLine();
                }
            }

            if (CollapsingHeader("Spinning Cube", ImGuiTreeNodeFlags_None)) {
                PushItemWidth(130.0f);
                SliderFloat("Metalness##2", &cube_metalness, 0.0f, 1.0f);
                SliderFloat("Roughness##2", &cube_roughness, 0.0f, 1.0f);
                PopItemWidth();
                Separator();

                Text("Rotation Mode");
                RadioButton("Local Space", &rotation_mode, 1); SameLine();
                RadioButton("World Space", &rotation_mode, 2);
                Separator();

                Text("Gizmo Edit Mode");
                RadioButton("T", &z_mode, 1); SameLine();
                RadioButton("R", &z_mode, 2); SameLine();
                RadioButton("S", &z_mode, 3); SameLine();
                RadioButton("N/A", &z_mode, 0);
                Separator();

                BeginGroup();
                for (int row = 0; row < 3; row++) {
                    for (int col = 0; col < 3; col++) {
                        int index = 3 * row + col;
                        ImVec2 alignment = ImVec2((float)col / 2.0f, (float)row / 2.0f);
                        PushStyleVar(ImGuiStyleVar_SelectableTextAlign, alignment);
                        PushStyleVar(ImGuiStyleVar_DisabledAlpha, 1.0f);

                        if (col > 0) SameLine();
                        if (cell_enabled[index] && !reset_cube) {
                            int direction = (index - 1) / 2;
                            bool active = cube_rotation == direction;
                            if (Selectable(cell_label[direction], active, 0, cell_size)) {
                                cube_rotation = active ? -1 : direction;
                            }
                        }
                        else if (row == 1 && col == 1) {
                            ui::PushRotation();
                            Selectable(ICON_FK_REPEAT, false, ImGuiSelectableFlags_Disabled, cell_size);
                            ui::PopRotation(Clock::time * 4.0f, false);
                        }
                        else {
                            Selectable("##empty", false, ImGuiSelectableFlags_Disabled, cell_size);
                        }

                        if (index == 5) {
                            if (SameLine(170.0f); Button("RESET", ImVec2(80.0f, 42.0f))) {
                                reset_cube = true;
                            }
                        }

                        PopStyleVar(2);
                    }
                }
                EndGroup();
            }

            if (CollapsingHeader("Color Torus", ImGuiTreeNodeFlags_None)) {
                PushItemWidth(130.0f);
                SliderFloat("Metalness##3", &torus_metalness, 0.00f, 1.0f);
                SliderFloat("Roughness##3", &torus_roughness, 0.01f, 1.0f);
                SliderFloat("Ambient Occlusion##3", &torus_ao, 0.05f, 0.5f);
                PopItemWidth();
                Checkbox("Torus Rotation", &rotate_torus);
            }

            if (CollapsingHeader("Motorbike", ImGuiTreeNodeFlags_None)) {
                PushItemWidth(130.0f);
                Checkbox("Wireframe Mode", &motor_wireframe);
                SliderFloat("Tank Roughness", &tank_roughness, 0.1f, 0.72f);
                PopItemWidth();
            }
            if (ImGui::TreeNode("Entity")) {
                static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Selected |
                    ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                static Entity e;
                for (auto& it : directory_Entity) {
                    ImGui::TreeNodeEx(it.name.c_str(), base_flags);
                    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
                        e = it;
                        CORE_INFO("{0} is selected", it.name);
                    }
                }
                if (e.id != entt::null) {
                    static int c = 0;
                    ImGui::RadioButton("T", &c, 0); ImGui::SameLine();
                    ImGui::RadioButton("R", &c, 1); ImGui::SameLine();
                    ImGui::RadioButton("S", &c, 2);

                    auto [x,y]=m_panelsManager->GetPanelAs<UI::Panels::AView>("Scene View").GetPosition();
                    auto [sx, sy] = m_panelsManager->GetPanelAs<UI::Panels::AView>("Scene View").GetSize();
                    ui::DrawGizmo(camera, e, c == 0 ? ui::Gizmo::Translate : c > 1 ? ui::Gizmo::Rotate : ui::Gizmo::Scale,std::make_pair(x,y),std::make_pair(sx,sy));
                }


                ImGui::TreePop();
            }
            Unindent(5.0f);
        }
    }

    void Scene02::Resize(int w, int h)
    {


        camera.GetComponent<Camera>().aspect = 1.0f * w / h;
        Scene::Resize(w, h); 
        FBOs.clear();
        AddFBO(w, h);
        AddFBO(w, h);
        AddFBO(w / 2, h / 2);

        FBOs[0].AddColorTexture(2, true);    // multisampled textures for MSAA
        FBOs[0].AddDepStRenderBuffer(true);  // multisampled RBO for MSAA
        FBOs[1].AddColorTexture(2);
        FBOs[2].AddColorTexture(2);

       

    }

    void Scene02::Present()
    {
        FBO& framebuffer_1 = FBOs[1];
        FBO& framebuffer_2 = FBOs[2];
        // ------------------------------ postprocessing pass ------------------------------

        framebuffer_1.GetColorTexture(0).Bind(0);  // color texture
        framebuffer_2.GetColorTexture(0).Bind(1);  // bloom texture

        auto bilinear_sampler = resource_manager.Get<Sampler>(StringToHash("BilinearSampler"));
        bilinear_sampler->Bind(1);  // upsample the bloom texture (bilinear filtering)

        auto postprocess_shader = resource_manager.Get<Shader>(StringToHash("post_process.glsl"));
        postprocess_shader->Bind();
        postprocess_shader->SetUniform(0, 3);  // select tone-mapping operator

        Renderer::Clear();
        Mesh::DrawQuad();

        postprocess_shader->Unbind();
        bilinear_sampler->Unbind(1);
    }


    void Scene02::PrecomputeIBL(const std::string& hdri) {
        Renderer::SeamlessCubemap(true);
        Renderer::DepthTest(false);
        Renderer::FaceCulling(true);

        auto irradiance_shader = CShader( "res\\shaders\\irradiance_map.glsl");
        auto prefilter_shader = CShader( "res\\shaders\\prefilter_envmap.glsl");
        auto envBRDF_shader = CShader( "res\\shaders\\environment_BRDF.glsl");

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

    void Scene02::SetupMaterial(Material& pbr_mat, int mat_id) {
        pbr_mat.SetTexture(pbr_t::irradiance_map, irradiance_map);
        pbr_mat.SetTexture(pbr_t::prefiltered_map, prefiltered_map);
        pbr_mat.SetTexture(pbr_t::BRDF_LUT, BRDF_LUT);

        pbr_mat.BindUniform(0, &skybox_exposure);

        if (mat_id == 0) {
            pbr_mat.SetUniform(pbr_u::albedo, vec4(1.0f));
            pbr_mat.SetUniform(pbr_u::metalness, 1.0f);
            pbr_mat.SetUniform(pbr_u::roughness, 0.0f);
        }
        else if (mat_id == 1) {
            std::string tex_path = "res\\texture\\brick_072\\";
            pbr_mat.SetTexture(pbr_t::albedo, MakeAsset<Texture>(tex_path + "albedo.jpg"));
            pbr_mat.SetTexture(pbr_t::normal, MakeAsset<Texture>(tex_path + "normal.jpg"));
            pbr_mat.SetTexture(pbr_t::roughness, MakeAsset<Texture>(tex_path + "roughness.jpg"));
            pbr_mat.SetTexture(pbr_t::ao, MakeAsset<Texture>(tex_path + "ao.jpg"));
            pbr_mat.SetUniform(pbr_u::metalness, 0.0f);
            pbr_mat.SetUniform(pbr_u::uv_scale, vec2(3.0f));
        }
        else if (mat_id == 2) {
            std::string tex_path =  "res\\texture\\marble_020\\";
            pbr_mat.SetTexture(pbr_t::albedo, MakeAsset<Texture>(tex_path + "albedo.jpg"));
            pbr_mat.SetTexture(pbr_t::roughness, MakeAsset<Texture>(tex_path + "roughness.jpg"));
            pbr_mat.SetUniform(pbr_u::metalness, 0.0f);
            pbr_mat.SetUniform(pbr_u::uv_scale, vec2(3.0f));
        }
        else if (mat_id >= 3 && mat_id <= 9) {
            unsigned int offset = mat_id - 3;
            sphere_color[offset] = vec4(utils::math::HSV2RGB(offset / 7.0f, 0.9f, 0.9f), 1.0f);
            pbr_mat.BindUniform(pbr_u::albedo, sphere_color + offset);
            pbr_mat.BindUniform(pbr_u::metalness, sphere_metalness + offset);
            pbr_mat.BindUniform(pbr_u::roughness, sphere_roughness + offset);
            pbr_mat.BindUniform(pbr_u::ao, &sphere_ao);
        }
        else if (mat_id >= 10 && mat_id <= 12) {
            pbr_mat.BindUniform(pbr_u::metalness, &cube_metalness);
            pbr_mat.BindUniform(pbr_u::roughness, &cube_roughness);
            pbr_mat.SetUniform(pbr_u::ao, 0.5f);
        }
        else if (mat_id == 20) {
            pbr_mat.BindUniform(pbr_u::albedo, &torus_color);
            pbr_mat.BindUniform(pbr_u::metalness, &torus_metalness);
            pbr_mat.BindUniform(pbr_u::roughness, &torus_roughness);
            pbr_mat.BindUniform(pbr_u::ao, &torus_ao);
        }
        else if (mat_id == 30) {  // motorbike
            pbr_mat.SetUniform(pbr_u::albedo, vec4(color::black, 1.0f));
            pbr_mat.SetUniform(pbr_u::metalness, 1.0f);
            pbr_mat.SetUniform(pbr_u::roughness, 0.62f);
        }
        else if (mat_id == 31) {
            pbr_mat.SetUniform(pbr_u::uv_scale, vec2(8.0f));
            pbr_mat.SetTexture(pbr_t::albedo, MakeAsset<Texture>("res\\model\\motorbike\\albedo.png"));
            pbr_mat.SetTexture(pbr_t::normal, MakeAsset<Texture>( "res\\model\\motorbike\\normal.png"));
            pbr_mat.SetTexture(pbr_t::roughness, MakeAsset<Texture>( "res\\model\\motorbike\\roughness.png"));
            pbr_mat.SetUniform(pbr_u::metalness, 0.0f);
        }
        else if (mat_id == 32) {
            pbr_mat.SetUniform(pbr_u::albedo, vec4(0.138f, 0.0f, 1.0f, 1.0f));
            pbr_mat.SetUniform(pbr_u::metalness, 1.0f);
            pbr_mat.BindUniform(pbr_u::roughness, &tank_roughness);
        }
        else if (mat_id == 33) {
            pbr_mat.SetUniform(pbr_u::albedo, vec4(color::black, 1.0f));
            pbr_mat.SetUniform(pbr_u::metalness, 0.0f);
            pbr_mat.SetUniform(pbr_u::roughness, 0.68f);
        }
        else if (mat_id == 34) {
            pbr_mat.SetUniform(pbr_u::albedo, vec4(0.28f, 0.28f, 0.28f, 1.0f));
            pbr_mat.SetUniform(pbr_u::metalness, 1.0f);
            pbr_mat.SetUniform(pbr_u::roughness, 0.62f);
        }
        else if (mat_id == 35) {
            pbr_mat.SetUniform(pbr_u::albedo, vec4(0.53f, 0.65f, 0.87f, 1.0f));
            pbr_mat.SetUniform(pbr_u::metalness, 1.0f);
            pbr_mat.SetUniform(pbr_u::roughness, 0.7f);
        }
        else if (mat_id == 36) {
            pbr_mat.SetUniform(pbr_u::albedo, vec4(0.4f, 0.4f, 0.4f, 1.0f));
            pbr_mat.SetUniform(pbr_u::metalness, 1.0f);
            pbr_mat.SetUniform(pbr_u::roughness, 0.72f);
        }
        else if (mat_id == 37) {
            pbr_mat.SetUniform(pbr_u::albedo, vec4(color::black, 1.0f));
            pbr_mat.SetUniform(pbr_u::metalness, 0.0f);
            pbr_mat.SetUniform(pbr_u::roughness, 0.76f);
            pbr_mat.SetTexture(pbr_t::normal, MakeAsset<Texture>( "res\\model\\motorbike\\normal22.png"));
        }
        else if (mat_id == 38) {
            pbr_mat.SetUniform(pbr_u::albedo, vec4(color::white, 1.0f));
            pbr_mat.SetUniform(pbr_u::metalness, 1.0f);
            pbr_mat.SetUniform(pbr_u::roughness, 0.0f);
        }
        else if (mat_id == 39) {
            pbr_mat.SetUniform(pbr_u::albedo, vec4(0.25f, 0.25f, 0.25f, 1.0f));
            pbr_mat.SetUniform(pbr_u::metalness, 1.0f);
            pbr_mat.SetUniform(pbr_u::roughness, 0.65f);
        }

        if (mat_id >= 30 && mat_id <= 39) {  // motorbike
            pbr_mat.BindUniform(1, &motor_wireframe);
        }
    }

    void Scene02::RenderSphere() {
        Renderer::Submit(sphere[0].id, sphere[1].id, sphere[2].id, sphere[3].id, sphere[4].id);
        Renderer::Submit(sphere[5].id, sphere[6].id, sphere[7].id, sphere[8].id, sphere[9].id);
    }

    void Scene02::RenderTorus() {
        float hue = math::Bounce(Clock::time * 0.05f, 1.0f);
        torus_color = vec4(math::HSV2RGB(vec3(hue, 1.0f, 1.0f)), 1.0f);

        if (rotate_torus) {
            torus.GetComponent<Transform>().Rotate(world::right, 0.36f, Space::Local);
        }

        Renderer::Submit(torus.id);
    }

    void Scene02::RenderCubes() {
        float delta_distance = cos(Clock::time * 1.5f) * 0.02f;
        cube[0].GetComponent<Transform>().Translate(delta_distance * world::up);
        cube[2].GetComponent<Transform>().Translate(delta_distance * world::down);

        auto& T = cube[1].GetComponent<Transform>();

        switch (cube_rotation) {
        case 00: T.Rotate(world::left, 0.5f, static_cast<Space>(rotation_mode)); break;
        case 01: T.Rotate(world::down, 0.5f, static_cast<Space>(rotation_mode)); break;
        case 02: T.Rotate(world::up, 0.5f, static_cast<Space>(rotation_mode)); break;
        case 03: T.Rotate(world::right, 0.5f, static_cast<Space>(rotation_mode)); break;
        case -1: default: break;
        }

        if (reset_cube) {
            const vec3 origin = vec3(0.0f, 5.0f, 0.0f);
            cube_rotation = -1;

            float t = math::EaseFactor(5.0f, Clock::delta_time);
            T.SetPosition(math::Lerp(T.position, origin, t));
            T.SetRotation(math::SlerpRaw(T.rotation, world::eye, t));

            if (math::Equals(T.position, origin) && math::Equals(T.rotation, world::eye)) {
                reset_cube = false;
            }
        }

        Renderer::Submit(cube[0].id, cube[1].id, cube[2].id);
    }

    void Scene02::RenderMotor() {
        Renderer::Submit(motorbike.id);
    }
}
