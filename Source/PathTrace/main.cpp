#define TESTMAIN
#ifdef TESTMAIN
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
#include"Panels/PanelsManager.h"
#include"Panels/AView.h"

#include"Opengl/core/log.h"
#include"ImGuizmo.h"
#include<string>
#include"Scene.h"
#include"LoadScene.h"
#include"Renderer.h"
using namespace std;
using namespace PathTrace;
float mouseSensitivity = 0.01f;
bool keyPressed = false;
Scene* scene = nullptr;
Renderer* renderer = nullptr;
std::vector<string> sceneFiles;
std::vector<string> envMaps;
int sampleSceneIdx = 0;
int selectedInstance = 0;
int envMapIdx = 0;

std::string shadersDir = "../../../res/PathTrace/shaders/";
std::string assetsDir = "../../../res/PathTrace/assets/";
std::string envMapDir = "../../../res/PathTrace/assets/HDR/";

RenderOptions renderOptions;

Scene* GetScene() {

    return scene;
}
Renderer* GetRenderer() {
    return renderer;
}
RenderOptions& GetRenderOptions() {
    return renderOptions;
}
void GetSceneFiles()
{
    std::filesystem::directory_entry p_directory(assetsDir);
    for (auto& item : std::filesystem::directory_iterator(p_directory))
        if (!item.is_directory()) {
            auto ext = item.path().extension();
            if (ext == ".scene")
            {
                sceneFiles.push_back(item.path().generic_string());
            }
        }
}
void GetEnvMaps()
{
    std::filesystem::directory_entry p_directory(envMapDir);
    for (auto& item : std::filesystem::directory_iterator(p_directory)) {
        if (item.path().extension() == ".hdr")
        {
            envMaps.push_back(item.path().generic_string());

        }
    }
}

void LoadScene(std::string sceneName)
{
    delete scene;
    scene = new Scene();
    std::string ext = sceneName.substr(sceneName.find_last_of(".") + 1);

    bool success = false;
    Mat4 xform;

    if (ext == "scene")
        success = LoadSceneFromFile(sceneName, scene, renderOptions);

    if (!success)
    {
        printf("Unable to load scene\n");
        exit(0);
    }

    selectedInstance = 0;
    // Add a default HDR if there are no lights in the scene
    if (!scene->envMap && !envMaps.empty())
    {
        scene->AddEnvMap(envMaps[envMapIdx]);
        renderOptions.enableEnvMap = scene->lights.empty() ? true : false;
        renderOptions.envMapIntensity = 1.5f;
    }

    scene->renderOptions = renderOptions;
}
bool InitRenderer()
{
    delete renderer;
    renderer = new Renderer(scene, shadersDir);
    return true;
}
double lastTime = 0.0;
bool done = false;
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw  %d: %s\n", error, description);
}


Windowing::Settings::WindowSettings windowSettings;
Windowing::Settings::DeviceSettings deviceSettings;
std::unique_ptr<Windowing::Context::Device>	device ;
std::unique_ptr<Windowing::Window> window ;
std::unique_ptr<Windowing::Inputs::InputManager>inputManager;
std::unique_ptr<UI::Core::UIManager>uiManager;
UI::Settings::PanelWindowSettings settings;
std::unique_ptr<PanelsManager>m_panelsManager;


void Render()
{
   GetRenderer()->Render();

}
void Update(float secondsElapsed)
{
    keyPressed = false;
    Scene* scene = GetScene();
    //相机视角交互逻辑
    if (m_panelsManager->GetPanelAs<AView>("Scene View").IsFocused() && ImGui::IsAnyMouseDown() && !ImGuizmo::IsOver())
    {
        if (ImGui::IsMouseDown(0))
        {
            ImVec2 mouseDelta = ImGui::GetMouseDragDelta(0, 0);
            scene->camera->OffsetOrientation(mouseDelta.x, mouseDelta.y);
            ImGui::ResetMouseDragDelta(0);
        }
        else if (ImGui::IsMouseDown(1))
        {
            ImVec2 mouseDelta = ImGui::GetMouseDragDelta(1, 0);
            scene->camera->SetRadius(mouseSensitivity * mouseDelta.y);
            ImGui::ResetMouseDragDelta(1);
        }
        else if (ImGui::IsMouseDown(2))
        {
            ImVec2 mouseDelta = ImGui::GetMouseDragDelta(2, 0);
            scene->camera->Strafe(mouseSensitivity * mouseDelta.x, mouseSensitivity * mouseDelta.y);
            ImGui::ResetMouseDragDelta(2);
        }
        scene->dirty = true;
    }

    GetRenderer()->Update(secondsElapsed);
}

void EditTransform(const float* view, const float* projection, float* matrix)
{
    static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
    static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);

    if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
    {
        mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
    }

    ImGui::SameLine();
    if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
    {
        mCurrentGizmoOperation = ImGuizmo::ROTATE;
    }

    ImGui::SameLine();
    if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
    {
        mCurrentGizmoOperation = ImGuizmo::SCALE;
    }

    float matrixTranslation[3], matrixRotation[3], matrixScale[3];
    ImGuizmo::DecomposeMatrixToComponents(matrix, matrixTranslation, matrixRotation, matrixScale);
    ImGui::InputFloat3("Tr", matrixTranslation);
    ImGui::InputFloat3("Rt", matrixRotation);
    ImGui::InputFloat3("Sc", matrixScale);
    ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, matrix);

    if (mCurrentGizmoOperation != ImGuizmo::SCALE)
    {
        if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
        {
            mCurrentGizmoMode = ImGuizmo::LOCAL;
        }

        ImGui::SameLine();
        if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
        {
            mCurrentGizmoMode = ImGuizmo::WORLD;
        }
    }

    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    ImGuizmo::Manipulate(view, projection, mCurrentGizmoOperation, mCurrentGizmoMode, matrix, NULL, NULL);
}
void MainLoop()
{

    Scene* scene = GetScene();
    //渲染ui
    ImGuizmo::SetOrthographic(false);

    ImGuizmo::BeginFrame();
    {
        ImGui::Begin("Settings");

        ImGui::Text("Samples: %d ", GetRenderer()->GetSampleCount());

        ImGui::BulletText("LMB + drag to rotate");
        ImGui::BulletText("MMB + drag to pan");
        ImGui::BulletText("RMB + drag to zoom in/out");
        ImGui::BulletText("CTRL + click on a slider to edit its value");

        if (ImGui::Button("Save Screenshot"))
        {
             //SaveFrame("./img_" + to_string(renderer->GetSampleCount()) + ".png");
        }

        // Scenes
        std::vector<const char*> scenes;
        for (int i = 0; i < sceneFiles.size(); ++i)
            scenes.push_back(sceneFiles[i].c_str());

        //끝쒼학뻣쭉서
        if (ImGui::Combo("Scene", &sampleSceneIdx, scenes.data(), scenes.size()))
        {
            int w = GetRenderOptions().windowResolution.x;
            int h = GetRenderOptions().windowResolution.y;

            LoadScene(sceneFiles[sampleSceneIdx]);
            GetRenderOptions().windowResolution.x = w;
            GetRenderOptions().windowResolution.y = h;
            InitRenderer();
        }

        // Environment maps
        std::vector<const char*> envMapsList;
        for (int i = 0; i < envMaps.size(); ++i)
            envMapsList.push_back(envMaps[i].c_str());

        if (ImGui::Combo("EnvMaps", &envMapIdx, envMapsList.data(), envMapsList.size()))
        {
            scene->AddEnvMap(envMaps[envMapIdx]);
        }

        bool optionsChanged = false;
        bool reloadShaders = false;

        optionsChanged |= ImGui::SliderFloat("Mouse Sensitivity", &mouseSensitivity, 0.001f, 1.0f);

        if (ImGui::CollapsingHeader("Render Settings"))
        {
            optionsChanged |= ImGui::SliderInt("Max Spp", &GetRenderOptions().maxSpp, -1, 256);
            optionsChanged |= ImGui::SliderInt("Max Depth", &GetRenderOptions().maxDepth, 1, 10);

            reloadShaders |= ImGui::Checkbox("Enable Russian Roulette", &GetRenderOptions().enableRR);
            reloadShaders |= ImGui::SliderInt("Russian Roulette Depth", &GetRenderOptions().RRDepth, 1, 10);
            reloadShaders |= ImGui::Checkbox("Enable Roughness Mollification", &renderOptions.enableRoughnessMollification);
            optionsChanged |= ImGui::SliderFloat("Roughness Mollification Amount", &renderOptions.roughnessMollificationAmt, 0, 1);
            reloadShaders |= ImGui::Checkbox("Enable Volume MIS", &renderOptions.enableVolumeMIS);
        }

        if (ImGui::CollapsingHeader("Environment"))
        {
            reloadShaders |= ImGui::Checkbox("Enable Uniform Light", &GetRenderOptions().enableUniformLight);

            Vec3 uniformLightCol = Vec3::Pow(GetRenderOptions().uniformLightCol, 1.0 / 2.2);
            optionsChanged |= ImGui::ColorEdit3("Uniform Light Color (Gamma Corrected)", (float*)(&uniformLightCol), 0);
            GetRenderOptions().uniformLightCol = Vec3::Pow(uniformLightCol, 2.2);

            reloadShaders |= ImGui::Checkbox("Enable Environment Map", &GetRenderOptions().enableEnvMap);
            optionsChanged |= ImGui::SliderFloat("Enviornment Map Intensity", &GetRenderOptions().envMapIntensity, 0.1f, 10.0f);
            optionsChanged |= ImGui::SliderFloat("Enviornment Map Rotation", &GetRenderOptions().envMapRot, 0.0f, 360.0f);
            reloadShaders |= ImGui::Checkbox("Hide Emitters", &GetRenderOptions().hideEmitters);
            reloadShaders |= ImGui::Checkbox("Enable Background", &GetRenderOptions().enableBackground);
            optionsChanged |= ImGui::ColorEdit3("Background Color", (float*)&GetRenderOptions().backgroundCol, 0);
            reloadShaders |= ImGui::Checkbox("Transparent Background", &GetRenderOptions().transparentBackground);
        }

        if (ImGui::CollapsingHeader("Tonemapping"))
        {
            ImGui::Checkbox("Enable Tonemap", &GetRenderOptions().enableTonemap);

            if (GetRenderOptions().enableTonemap)
            {
                ImGui::Checkbox("Enable ACES", &GetRenderOptions().enableAces);
                if (GetRenderOptions().enableAces)
                    ImGui::Checkbox("Simple ACES Fit", &GetRenderOptions().simpleAcesFit);
            }
        }

        if (ImGui::CollapsingHeader("Denoiser"))
        {

            ImGui::Checkbox("Enable Denoiser", &GetRenderOptions().enableDenoiser);
            ImGui::SliderInt("Number of Frames to skip", &GetRenderOptions().denoiserFrameCnt, 5, 50);
        }

        if (ImGui::CollapsingHeader("Camera"))
        {
            float fov = Math::Degrees(scene->camera->fov);
            float aperture = scene->camera->aperture * 1000.0f;
            optionsChanged |= ImGui::SliderFloat("Fov", &fov, 10, 90);
            scene->camera->SetFov(fov);
            optionsChanged |= ImGui::SliderFloat("Aperture", &aperture, 0.0f, 10.8f);
            scene->camera->aperture = aperture / 1000.0f;
            optionsChanged |= ImGui::SliderFloat("Focal Distance", &scene->camera->focalDist, 0.01f, 50.0f);
            ImGui::Text("Pos: %.2f, %.2f, %.2f", scene->camera->position.x, scene->camera->position.y, scene->camera->position.z);
        }

        if (ImGui::CollapsingHeader("Objects"))
        {
            bool objectPropChanged = false;

            std::vector<std::string> listboxItems;
            for (int i = 0; i < scene->meshInstances.size(); i++)
            {
                listboxItems.push_back(scene->meshInstances[i].name);
            }

            // Object Selection
            if (ImGui::ListBoxHeader("Instances")) {
                for (int i = 0; i < scene->meshInstances.size(); i++)
                {
                    bool is_selected = selectedInstance == i;
                    if (ImGui::Selectable(listboxItems[i].c_str(), is_selected))
                    {
                        selectedInstance = i;
                    }
                }
                ImGui::ListBoxFooter();
            }
            ImGui::Separator();
            ImGui::Text("Materials");
            // Material Properties
            Material* mat = &scene->materials[scene->meshInstances[selectedInstance].materialID];
            // Gamma correction for color picker. Internally, the renderer uses linear RGB values for colors
            Vec3 albedo = Vec3::Pow(mat->baseColor, 1.0 / 2.2);
            objectPropChanged |= ImGui::ColorEdit3("Albedo (Gamma Corrected)", (float*)(&albedo), 0);
            mat->baseColor = Vec3::Pow(albedo, 2.2);

            objectPropChanged |= ImGui::SliderFloat("Metallic", &mat->metallic, 0.0f, 1.0f);
            objectPropChanged |= ImGui::SliderFloat("Roughness", &mat->roughness, 0.001f, 1.0f);
            objectPropChanged |= ImGui::SliderFloat("SpecularTint", &mat->specularTint, 0.0f, 1.0f);
            objectPropChanged |= ImGui::SliderFloat("Subsurface", &mat->subsurface, 0.0f, 1.0f);
            objectPropChanged |= ImGui::SliderFloat("Anisotropic", &mat->anisotropic, 0.0f, 1.0f);
            objectPropChanged |= ImGui::SliderFloat("Sheen", &mat->sheen, 0.0f, 1.0f);
            objectPropChanged |= ImGui::SliderFloat("SheenTint", &mat->sheenTint, 0.0f, 1.0f);
            objectPropChanged |= ImGui::SliderFloat("Clearcoat", &mat->clearcoat, 0.0f, 1.0f);
            objectPropChanged |= ImGui::SliderFloat("ClearcoatGloss", &mat->clearcoatGloss, 0.0f, 1.0f);
            objectPropChanged |= ImGui::SliderFloat("SpecTrans", &mat->specTrans, 0.0f, 1.0f);
            objectPropChanged |= ImGui::SliderFloat("Ior", &mat->ior, 1.001f, 2.0f);

            int mediumType = (int)mat->mediumType;
            if (ImGui::Combo("Medium Type", &mediumType, "None\0Absorb\0Scatter\0Emissive\0"))
            {
                reloadShaders = true;
                objectPropChanged = true;
                mat->mediumType = mediumType;
            }

            if (mediumType != MediumType::None)
            {
                Vec3 mediumColor = Vec3::Pow(mat->mediumColor, 1.0 / 2.2);
                objectPropChanged |= ImGui::ColorEdit3("Medium Color (Gamma Corrected)", (float*)(&mediumColor), 0);
                mat->mediumColor = Vec3::Pow(mediumColor, 2.2);

                objectPropChanged |= ImGui::SliderFloat("Medium Density", &mat->mediumDensity, 0.0f, 5.0f);

                if (mediumType == MediumType::Scatter)
                    objectPropChanged |= ImGui::SliderFloat("Medium Anisotropy", &mat->mediumAnisotropy, -0.9f, 0.9f);
            }

            int alphaMode = (int)mat->alphaMode;
            if (ImGui::Combo("Alpha Mode", &alphaMode, "Opaque\0Blend"))
            {
                reloadShaders = true;
                objectPropChanged = true;
                mat->alphaMode = alphaMode;
            }

            if (alphaMode != AlphaMode::Opaque)
                objectPropChanged |= ImGui::SliderFloat("Opacity", &mat->opacity, 0.0f, 1.0f);

            // Transforms
            ImGui::Separator();
            ImGui::Text("Transforms");
            {
                float viewMatrix[16];
                float projMatrix[16];

                auto io = ImGui::GetIO();
                scene->camera->ComputeViewProjectionMatrix(viewMatrix, projMatrix, io.DisplaySize.x / io.DisplaySize.y);
                Mat4 xform = scene->meshInstances[selectedInstance].transform;

                EditTransform(viewMatrix, projMatrix, (float*)&xform);

                if (memcmp(&xform, &scene->meshInstances[selectedInstance].transform, sizeof(float) * 16))
                {
                    scene->meshInstances[selectedInstance].transform = xform;
                    objectPropChanged = true;
                }
            }

            if (objectPropChanged)
                scene->RebuildInstances();
        }

        scene->renderOptions = renderOptions;

        if (optionsChanged)
            scene->dirty = true;

        if (reloadShaders)
        {
            scene->dirty = true;
            renderer->ReloadShaders();
        }

        ImGui::End();


    }
    static double lasttime = glfwGetTime();
    static double curtime = glfwGetTime();

    curtime = glfwGetTime();


    Update((float)(curtime - lasttime));
    lastTime = curtime;

    Render();

}

int main(int, char**)
{
    {

    deviceSettings.contextMajorVersion = 4;
    deviceSettings.contextMinorVersion = 6;
    windowSettings.title = "PathTrace";
    windowSettings.width = renderOptions.windowResolution.x;
    windowSettings.height = renderOptions.windowResolution.y;
    windowSettings.maximized = true;
  	device = std::make_unique<Windowing::Context::Device>(deviceSettings);
    window = std::make_unique<Windowing::Window>(*device, windowSettings);
    window->SetIcon("../../../res/texture/awesomeface.png");
    inputManager= std::make_unique<Windowing::Inputs::InputManager>(*window);;
    window->MakeCurrentContext();

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        return -1;
    }
    device->SetVsync(true);


    uiManager=std::make_unique<UI::Core::UIManager>(window->GetGlfwWindow(), UI::Styling::EStyle::CUSTOM);;
    uiManager->LoadFont("Ruda_Big",  "../../../res/font/Ruda-Bold.ttf", 18);
    uiManager->LoadFont("Ruda_Small", "../../../res/font/Ruda-Bold.ttf", 12);
    uiManager->LoadFont("Ruda_Medium",  "../../../res/font/Ruda-Bold.ttf", 14);
    uiManager->UseFont("Ruda_Big");
    uiManager->SetEditorLayoutSaveFilename(std::string(getenv("APPDATA")) + "\\OverloadTech\\OvEditor\\layout.ini");
    uiManager->SetEditorLayoutAutosaveFrequency(60.0f);
    uiManager->EnableEditorLayoutSave(true);
    uiManager->EnableDocking(true);

    settings.closable = true;
    settings.collapsable = true;
    settings.dockable = true;
    UI::Modules::Canvas m_canvas;
    m_panelsManager= std::make_unique<PanelsManager>(m_canvas);

    m_panelsManager->CreatePanel<MenuBar>("Menu Bar");
    m_panelsManager->CreatePanel<AView>("Scene View", true, settings);
    m_panelsManager->GetPanelAs<AView>("Scene View").ResizeEvent+= [](int p_width, int p_height) {
        renderOptions.windowResolution.x = p_width;
        renderOptions.windowResolution.y = p_height;
        if (!renderOptions.independentRenderSize)
            renderOptions.renderResolution = renderOptions.windowResolution;
        scene->renderOptions = renderOptions;
        renderer->ResizeRenderer();
    };

    m_canvas.MakeDockspace(true);
    uiManager->SetCanvas(m_canvas);

    //初始化场景
    ::core::Log::Init();
    GetSceneFiles();
    GetEnvMaps();
    LoadScene(sceneFiles[sampleSceneIdx]);
    if (!InitRenderer())
        return 1;
    //主循环
    Tools::Time::Clock clock;
    while (!window->ShouldClose())
    {
        glClearColor(0., 0., 0., 0.);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        MainLoop();
        
        uiManager->Render();
        m_panelsManager->GetPanelAs<AView>("Scene View").Update(1);
        m_panelsManager->GetPanelAs<AView>("Scene View").Bind();
        
        
        renderer->Present();
        m_panelsManager->GetPanelAs<AView>("Scene View").UnBind();

        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        device->PollEvents();
        window->SwapBuffers();
        inputManager->ClearEvents();
        clock.Update();
    }
    //回收资源
    delete renderer;
    delete scene;
    device.reset();
    ::core::Log::Shutdown();
    uiManager.reset();
    m_panelsManager.reset();
    inputManager.reset();
    window.reset();
    } 
    return 0;
}
#endif // TESTMAIN