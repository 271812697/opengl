#include "scene_03.h"

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
        windowSettings.title = "Disney Principled BSDF";
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
        scene::Scene* S = new scene::Scene03("Disney Principled BSDF");
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

    static bool  show_grid       = false;
    static float grid_cell_size  = 2.0f;
    static vec4  thin_line_color = vec4(0.1f, 0.1f, 0.1f, 1.0f);
    static vec4  wide_line_color = vec4(0.2f, 0.2f, 0.2f, 1.0f);
    static vec3  dl_direction    = vec3(0.7f, -0.7f, 0.0f);

    static float skybox_exposure = 1.0f;
    static float skybox_lod      = 0.0f;

    static bool  show_gizmo    = false;
    static bool  rotate_model  = false;
    static bool  reset_model   = false;
    static vec3  model_axis    = world::right;

    static float const_zero    = 0.0f;
    static int   entity_id     = 1;
    static uint  shading_model = 11;

    static vec4  albedo        = vec4(color::white, 1.0f);
    static float roughness     = 1.0f;
    static float ao            = 1.0f;
    static float metalness     = 0.0f;
    static float specular      = 0.5f;
    static float anisotropy    = 0.0f;
    static vec3  aniso_dir     = world::right;
    static float transmission  = 0.0f;
    static float thickness     = 2.0f;
    static float ior           = 1.5f;
    static vec3  transmittance = color::purple;
    static float tr_distance   = 4.0f;
    static uint  volume_type   = 0U;
    static float clearcoat     = 0.0f;
    static float cc_roughness  = 0.0f;

    ///////////////////////////////////////////////////////////////////////////////////////////////

    void Scene03::Init() {
        Renderer::SetScene(this);
        this->title = "Disney Principled BSDF";
        PrecomputeIBL("res\\texture\\HDRI\\outdoor.hdr");

        resource_manager.Add(01, MakeAsset<Shader>("res\\shaders\\infinite_grid.glsl"));
        resource_manager.Add(02, MakeAsset<Shader>( "res\\shaders\\skybox.glsl"));
        resource_manager.Add(04, MakeAsset<Shader>(  "res\\shaders\\pbr03.glsl"));
        resource_manager.Add(05, MakeAsset<Shader>( "res\\shaders\\post_process03.glsl"));
        resource_manager.Add(12, MakeAsset<Material>(resource_manager.Get<Shader>(02)));
        resource_manager.Add(14, MakeAsset<Material>(resource_manager.Get<Shader>(04)));
        
        AddUBO(resource_manager.Get<Shader>(02)->ID());
        AddUBO(resource_manager.Get<Shader>(04)->ID());



        camera = CreateEntity("Camera", ETag::MainCamera);
        camera.GetComponent<Transform>().Translate(0.0f, 6.0f, 9.0f);
        camera.AddComponent<Camera>(View::Perspective);
        camera.AddComponent<Spotlight>(color::red, 3.8f);
        camera.GetComponent<Spotlight>().SetCutoff(4.0f, 10.0f, 45.0f);
        
        skybox = CreateEntity("Skybox", ETag::Skybox);
        skybox.AddComponent<Mesh>(Primitive::Cube);
        if (auto& mat = skybox.AddComponent<Material>(resource_manager.Get<Material>(12)); true) {
            mat.SetTexture(0, prefiltered_map);
            mat.BindUniform(0, &skybox_exposure);
            mat.BindUniform(1, &skybox_lod);
        }

        direct_light = CreateEntity("Directional Light");
        direct_light.GetComponent<Transform>().Rotate(45.0f, 180.0f, 0.0f, Space::World);
        direct_light.AddComponent<DirectionLight>(color::yellow, 0.2f);  // 0 attenuation -> small intensity

        pistol = CreateEntity("Pistol");
        pistol.GetComponent<Transform>().Translate(vec3(0.0f, 5.0f, 0.0f));
        pistol.GetComponent<Transform>().Scale(0.3f);

        if (std::string m_path = "res\\model\\SW500\\"; true) {
            auto& model = pistol.AddComponent<Model>(m_path + "SW500.fbx", Quality::Auto);
            auto& mat_b = model.SetMaterial("TEX_Bullet",  resource_manager.Get<Material>(14));
            auto& mat_p = model.SetMaterial("TEX_Lowpoly", resource_manager.Get<Material>(14));

            SetupMaterial(mat_b);
            mat_b.SetUniform(pbr_u::shading_model, uvec2(1, 0));  // bullet ignores clear coat layer
            mat_b.SetTexture(pbr_t::albedo,    MakeAsset<Texture>(m_path + "bullet_albedo.jpg"));
            mat_b.SetTexture(pbr_t::normal,    MakeAsset<Texture>(m_path + "bullet_normal.png"));
            mat_b.SetTexture(pbr_t::metallic,  MakeAsset<Texture>(m_path + "bullet_metallic.jpg"));
            mat_b.SetTexture(pbr_t::roughness, MakeAsset<Texture>(m_path + "bullet_roughness.jpg"));
            mat_b.SetTexture(pbr_t::ao,        MakeAsset<Texture>(m_path + "bullet_AO.jpg"));

            SetupMaterial(mat_p);
            mat_p.SetUniform(pbr_u::shading_model, uvec2(1, 1));
            mat_p.SetTexture(pbr_t::albedo,    MakeAsset<Texture>(m_path + "SW500_albedo.png"));
            mat_p.SetTexture(pbr_t::normal,    MakeAsset<Texture>(m_path + "SW500_normal.png"));
            mat_p.SetTexture(pbr_t::metallic,  MakeAsset<Texture>(m_path + "SW500_metallic.png"));
            mat_p.SetTexture(pbr_t::roughness, MakeAsset<Texture>(m_path + "SW500_roughness.png"));
            mat_p.SetTexture(pbr_t::ao,        MakeAsset<Texture>(m_path + "SW500_AO.jpg"));
        }

        helmet = CreateEntity("Helmet");
        helmet.GetComponent<Transform>().Translate(vec3(0.2f, 4.0f, -2.0f));
        helmet.GetComponent<Transform>().Scale(0.02f);

        if (auto& model = helmet.AddComponent<Model>("res\\model\\mandalorian.fbx", Quality::Auto); true) {
            SetupMaterial(model.SetMaterial("DefaultMaterial", resource_manager.Get<Material>(14)));
            SetupMaterial(model.SetMaterial("Material #26", resource_manager.Get<Material>(14)));
        }

        pyramid = CreateEntity("Pyramid");
        pyramid.AddComponent<Mesh>(Primitive::Tetrahedron);
        pyramid.GetComponent<Transform>().Translate(world::up * 5.0f);
        pyramid.GetComponent<Transform>().Scale(2.0f);

        if (auto& mat = pyramid.AddComponent<Material>(resource_manager.Get<Material>(14)); true) {
            SetupMaterial(mat);
            mat.SetUniform(pbr_u::shading_model, uvec2(2, 0));
        }

        capsule = CreateEntity("Capsule");
        capsule.AddComponent<Mesh>(Primitive::Capsule);
        capsule.GetComponent<Transform>().Translate(world::up * 5.0f);
        capsule.GetComponent<Transform>().Scale(2.0f);

        if (auto& mat = capsule.AddComponent<Material>(resource_manager.Get<Material>(14)); true) {
            SetupMaterial(mat);
            mat.SetUniform(pbr_u::shading_model, uvec2(2, 0));
        }

        Renderer::MSAA(true);
        Renderer::DepthTest(true);
        Renderer::AlphaBlend(true);
    }

    Entity& Scene03::GetEntity(int entity_id) {


        switch (entity_id) {
            case 1: return pistol;
            case 2: return helmet;
            case 3: return pyramid;
            case 4: return capsule;
            default: throw std::runtime_error("Invalid entity id!");
        }
    }

    void Scene03::SetSelected_Entity()
    {
        auto call = [this](const Entity& val) {
            for (int i = 0; i < directory_Entity.size(); i++) {
                if (directory_Entity[i].id == val.id) {
                    return i;
                }
            }
            return -1;
        };
        switch (entity_id) {
        case 1: selected_entity = call(pistol); break;
        case 2: selected_entity = call(helmet); break;
        case 3: selected_entity = call(pyramid); break;
        case 4:selected_entity = call(capsule); break;
        default: throw std::runtime_error("Invalid entity id!");
        }
    }

    void Scene03::OnSceneRender(float dt) {
        auto& e = GetEntity(entity_id);
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
            auto& dl = direct_light.GetComponent<DirectionLight>();
            vec3 direction = -glm::normalize(dl_direction);
            ubo.SetUniform(0, val_ptr(dl.color));
            ubo.SetUniform(1, val_ptr(direction));
            ubo.SetUniform(2, val_ptr(dl.intensity));
        }

        if (auto& ubo = UBOs[2]; true) {
            auto& sl = camera.GetComponent<Spotlight>();
            auto& ct = camera.GetComponent<Transform>();
            float inner_cos = sl.GetInnerCosine();
            float outer_cos = sl.GetOuterCosine();
            ubo.SetUniform(0, val_ptr(sl.color));
            ubo.SetUniform(1, val_ptr(ct.position));
            ubo.SetUniform(2, val_ptr(-ct.forward));
            ubo.SetUniform(3, val_ptr(sl.intensity));
            ubo.SetUniform(4, val_ptr(inner_cos));
            ubo.SetUniform(5, val_ptr(outer_cos));
            ubo.SetUniform(6, val_ptr(sl.range));
        }

        FBO& framebuffer_0 = FBOs[0];
        FBO& framebuffer_1 = FBOs[1];

        // ------------------------------ MRT render pass ------------------------------

        framebuffer_0.Clear();
        framebuffer_0.Bind();

        if (reset_model) {
            const vec3 origin = vec3(0.0f, 5.0f, 0.0f);
            auto& T = e.GetComponent<Transform>();
            float t = math::EaseFactor(5.0f, Clock::delta_time);
            T.SetPosition(math::Lerp(T.position, origin, t));
            T.SetRotation(math::SlerpRaw(T.rotation, world::eye, t));

            if (math::Equals(T.position, origin) && math::Equals(T.rotation, world::eye)) {
                reset_model = false;
            }
        }
        else if (rotate_model) {
            e.GetComponent<Transform>().Rotate(model_axis, 0.36f, Space::Local);
        }

        Renderer::FaceCulling(entity_id != 2);
        Renderer::Submit(e.id);
        Renderer::Submit(skybox.id);
        Renderer::Render();

        if (show_grid) {
            auto grid_shader = resource_manager.Get<Shader>(01);
            grid_shader->Bind();
            grid_shader->SetUniform(0, grid_cell_size);
            grid_shader->SetUniform(1, thin_line_color);
            grid_shader->SetUniform(2, wide_line_color);
            Mesh::DrawGrid();
        }

        framebuffer_0.Unbind();

        // ------------------------------ MSAA resolve pass ------------------------------
        
        framebuffer_1.Clear();
        FBO::CopyColor(framebuffer_0, 0, framebuffer_1, 0);


    }
    void Scene03::Resize(int w, int h) {
        camera.GetComponent<Camera>().aspect = 1.0f * w / h;
        Scene::Resize(w, h);
        FBOs.clear();
        AddFBO(w, h);
        AddFBO(w, h);

        FBOs[0].AddColorTexture(1, true);
        FBOs[0].AddDepStRenderBuffer(true);
        FBOs[1].AddColorTexture(1);
    }

    void Scene03::Present() {
        // ------------------------------ postprocessing pass ------------------------------
        FBO& framebuffer_1 = FBOs[1];
        framebuffer_1.GetColorTexture(0).Bind(0);
        auto postprocess_shader = resource_manager.Get<Shader>(05);
        postprocess_shader->Bind();
        postprocess_shader->SetUniform(0, 3);

        Renderer::Clear();
        Mesh::DrawQuad();
        postprocess_shader->Unbind();
    }

    void Scene03::OnImGuiRender(float dt ) {
        using namespace ImGui;
        const ImGuiColorEditFlags color_flags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha;
        const ImVec2 rainbow_offset = ImVec2(5.0f, 105.0f);
        const ImVec4 tab_color_off  = ImVec4(0.0f, 0.3f, 0.6f, 1.0f);
        const ImVec4 tab_color_on   = ImVec4(0.0f, 0.4f, 0.8f, 1.0f);

        if (true) {
            Indent(5.0f);
            Text("  Directional Light Vector");
            DragFloat3("###", val_ptr(dl_direction), 0.01f, -1.0f, 1.0f, "%.3f");
           
            Spacing();

            PushItemWidth(130.0f);
            SliderFloat("Skybox Exposure", &skybox_exposure, 0.5f, 4.0f);
            SliderFloat("Skybox LOD", &skybox_lod, 0.0f, 7.0f);
            PopItemWidth();
            Checkbox("Gizmo", &show_gizmo); SameLine();
            reset_model |= Button("###", ImVec2(30.0f, 0.0f)); SameLine();
            Text("Reset"); SameLine();
            if (Checkbox("Rotation", &rotate_model); rotate_model) {
                DragFloat3("Local Axis", val_ptr(model_axis), 0.01f, 0.0f, 1.0f, "%.2f");
            }
            Separator();

            BeginTabBar("InspectorTab", ImGuiTabBarFlags_None);

            if (BeginTabItem("ClearCoat")) {
                if (entity_id != 1) {
                    entity_id = 1;
                    anisotropy = 0.0f;
                }
                PushItemWidth(130.0f);
                SliderFloat("Specular", &specular, 0.35f, 1.0f);
                SliderFloat("Clearcoat", &clearcoat, 0.0f, 1.0f);
                SliderFloat("Clearcoat Roughness", &cc_roughness, 0.045f, 1.0f);
                PopItemWidth();
                EndTabItem();
            }

            if (BeginTabItem("Anisotropy")) {
                if (entity_id != 2) {
                    entity_id = 2;
                    metalness = roughness = 1.0f;
                }
                PushItemWidth(130.0f);
                ColorEdit4("Albedo", val_ptr(albedo), ImGuiColorEditFlags_NoInputs);
                SliderFloat("Roughness", &roughness, 0.045f, 1.0f);
                SliderFloat("Ambient Occlusion", &ao, 0.05f, 1.0f);
                SliderFloat("Anisotropy", &anisotropy, -1.0f, 1.0f);
                DragFloat3("Anisotropy Direction", val_ptr(aniso_dir), 0.01f, 0.1f, 1.0f, "%.1f");
                PopItemWidth();
                EndTabItem();
            }

            if (BeginTabItem("Refraction")) {
                if (entity_id < 3) {
                    entity_id = std::max(entity_id, 3);
                    roughness = 0.2f;
                }
                RadioButton("Cubic/Flat", &entity_id, 3); SameLine(164);
                RadioButton("Spherical", &entity_id, 4);
                volume_type = entity_id == 4 ? 0U : 1U;
                
                PushItemWidth(130.0f);
                ColorEdit4("Albedo", val_ptr(albedo), ImGuiColorEditFlags_NoInputs); SameLine(164);
                ColorEdit4("Transmittance", val_ptr(transmittance), ImGuiColorEditFlags_NoInputs);
                SliderFloat("Roughness", &roughness, 0.045f, 1.0f);
                SliderFloat("Ambient Occlusion", &ao, 0.05f, 1.0f);
                SliderFloat("Transmission", &transmission, 0.0f, 1.0f);
                SliderFloat("Thickness", &thickness, 2.0f, 4.0f);
                SliderFloat("IOR", &ior, 1.0f, 1.5f);
                SliderFloat("Transmission Distance", &tr_distance, 0.0f, 4.0f);
                PopItemWidth();
                EndTabItem();
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

        if (show_gizmo) {
           // ui::DrawGizmo(camera, GetEntity(entity_id), ui::Gizmo::Translate);
            SetSelected_Entity();
        }
        else
        {
            selected_entity = -1;
        }
    }

    void Scene03::PrecomputeIBL(const std::string& hdri) {
        Renderer::SeamlessCubemap(true);
        Renderer::DepthTest(false);
        Renderer::FaceCulling(true);

        auto irradiance_shader = CShader( "res\\shaders\\irradiance_map.glsl");
        auto prefilter_shader  = CShader( "res\\shaders\\prefilter_envmap.glsl");
        auto envBRDF_shader    = CShader("res\\shaders\\environment_BRDF.glsl");

        auto env_map = MakeAsset<Texture>(hdri, 2048, 0);
        env_map->Bind(0);

        irradiance_map  = MakeAsset<Texture>(GL_TEXTURE_CUBE_MAP, 128, 128, 6, GL_RGBA16F, 1);
        prefiltered_map = MakeAsset<Texture>(GL_TEXTURE_CUBE_MAP, 2048, 2048, 6, GL_RGBA16F, 8);
        BRDF_LUT        = MakeAsset<Texture>(GL_TEXTURE_2D, 1024, 1024, 1, GL_RGBA16F, 1);

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

    void Scene03::SetupMaterial(Material& mat) {
        mat.SetTexture(pbr_t::irradiance_map, irradiance_map);
        mat.SetTexture(pbr_t::prefiltered_map, prefiltered_map);
        mat.SetTexture(pbr_t::BRDF_LUT, BRDF_LUT);

        mat.BindUniform(0, &skybox_exposure);
        mat.BindUniform(pbr_u::albedo, &albedo);
        mat.BindUniform(pbr_u::roughness, &roughness);
        mat.BindUniform(pbr_u::ao, &ao);
        mat.BindUniform(pbr_u::metalness, &metalness);
        mat.BindUniform(pbr_u::specular, &specular);
        mat.BindUniform(pbr_u::anisotropy, &anisotropy);
        mat.BindUniform(pbr_u::aniso_dir, &aniso_dir);
        mat.BindUniform(pbr_u::transmission, &transmission);
        mat.BindUniform(pbr_u::thickness, &thickness);
        mat.BindUniform(pbr_u::ior, &ior);
        mat.BindUniform(pbr_u::transmittance, &transmittance);
        mat.BindUniform(pbr_u::tr_distance, &tr_distance);
        mat.BindUniform(pbr_u::volume_type, &volume_type);
        mat.BindUniform(pbr_u::clearcoat, &clearcoat);
        mat.BindUniform(pbr_u::cc_roughness, &cc_roughness);
    }

}
