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
#include"UI/Panels/PanelsManager.h"
#include"UI/Panels/AView.h"
#include "Panels/PathInspector.h"
#include"Opengl/core/log.h"
#include"ImGuizmo.h"
#include<string>
#include"LoadScene.h"
#include"Renderer.h"
#include"PathTrace.h"
using namespace std;
using namespace PathTrace;

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
std::unique_ptr<UI::Panels::PanelsManager>m_panelsManager;

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
int main(int, char**)
{
    {

    deviceSettings.contextMajorVersion = 4;
    deviceSettings.contextMinorVersion = 6;
    windowSettings.title = "PathTrace";
    windowSettings.width = GetRenderOptions().windowResolution.x;
    windowSettings.height = GetRenderOptions().windowResolution.y;
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
    //初始化场景
    ::core::Log::Init();
    GetSceneFiles();
    GetEnvMaps();
    LoadScene(sceneFiles[sampleSceneIdx]);
    if (!InitRenderer())
        return 1;
    //初始化UI
    uiManager=std::make_unique<UI::Core::UIManager>(window->GetGlfwWindow(), UI::Styling::EStyle::CUSTOM);;
    uiManager->LoadFont("Ruda_Big",  "../../../res/font/Ruda-Bold.ttf", 18);
    uiManager->LoadFont("Ruda_Small", "../../../res/font/Ruda-Bold.ttf", 12);
    uiManager->LoadFont("Ruda_Medium",  "../../../res/font/Ruda-Bold.ttf", 14);
    uiManager->UseFont("Ruda_Big");

    uiManager->EnableDocking(true);

    settings.closable = true;
    settings.collapsable = true;
    settings.dockable = true;
    UI::Modules::Canvas m_canvas;
    m_panelsManager= std::make_unique<UI::Panels::PanelsManager>(m_canvas);

    m_panelsManager->CreatePanel<UI::Panels::MenuBar>("Menu Bar");
    m_panelsManager->CreatePanel<PathInspector>("Inspector",true, settings);
    m_panelsManager->GetPanelAs<PathInspector>("Inspector").InstallUI();
    m_panelsManager->CreatePanel<UI::Panels::AView>("Scene View", true, settings);
    m_panelsManager->GetPanelAs<UI::Panels::AView>("Scene View").ResizeEvent+= [](int p_width, int p_height) {
        GetRenderOptions().windowResolution.x = p_width;
        GetRenderOptions().windowResolution.y = p_height;
        if (!GetRenderOptions().independentRenderSize)
            GetRenderOptions().renderResolution = GetRenderOptions().windowResolution;
        GetScene()->renderOptions = GetRenderOptions();
        GetRenderer()->ResizeRenderer();
    };
    m_panelsManager->GetPanelAs<UI::Panels::AView>("Scene View").DrawInWindow += []() {
 
        if (showTransform) {
        auto pos = m_panelsManager->GetPanelAs<UI::Panels::AView>("Scene View").GetPosition();
        auto size = m_panelsManager->GetPanelAs<UI::Panels::AView>("Scene View").GetSize();

        {
            float viewMatrix[16];
            float projMatrix[16];
           
            GetScene()->camera->ComputeViewProjectionMatrix(viewMatrix, projMatrix, size.x / size.y);
            Mat4 xform = GetScene()->meshInstances[selectedInstance].transform;
           
            ImGuizmo::SetRect(pos.x, pos.y, size.x, size.y);
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();
            ImGuizmo::Manipulate(viewMatrix, projMatrix, ImGuizmo::UNIVERSAL, ImGuizmo::LOCAL, (float*)&xform, NULL, NULL);
      
            if (ImGuizmo::IsUsing())
            {
                GetScene()->meshInstances[selectedInstance].transform = xform;
                objectPropChanged = true;
            }
        }
        }
    };

    m_canvas.MakeDockspace(true);
    uiManager->SetCanvas(m_canvas);
    //主循环
    Tools::Time::Clock clock;
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    while (!window->ShouldClose())
    {
        glClearColor(0., 0., 0., 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glDisable(GL_DEPTH_TEST);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
       
        //相机视角交互逻辑
        if (m_panelsManager->GetPanelAs<UI::Panels::AView>("Scene View").IsFocused() && ImGui::IsAnyMouseDown() && !ImGuizmo::IsOver())
        {

            if (ImGui::IsMouseDown(0))
            {
                ImVec2 mouseDelta = ImGui::GetMouseDragDelta(0, 0);
                GetScene()->camera->OffsetOrientation(mouseDelta.x, mouseDelta.y);
                ImGui::ResetMouseDragDelta(0);
            }
            else if (ImGui::IsMouseDown(1))
            {
                ImVec2 mouseDelta = ImGui::GetMouseDragDelta(1, 0);
                GetScene()->camera->SetRadius(mouseSensitivity * mouseDelta.y);
                ImGui::ResetMouseDragDelta(1);
            }
            else if (ImGui::IsMouseDown(2))
            {
                ImVec2 mouseDelta = ImGui::GetMouseDragDelta(2, 0);
                GetScene()->camera->Strafe(mouseSensitivity * mouseDelta.x, mouseSensitivity * mouseDelta.y);
                ImGui::ResetMouseDragDelta(2);
            }

            GetScene()->dirty = true;
        }
        GetRenderer()->Update(clock.GetDeltaTime());

        GetRenderer()->Render();
        //GetRenderer()->RenderPBR();
        m_panelsManager->GetPanelAs<UI::Panels::AView>("Scene View").Update(1);
        m_panelsManager->GetPanelAs<UI::Panels::AView>("Scene View").Bind();
        GetRenderer()->Present();
        //GetRenderer()->PresentPBR();
        m_panelsManager->GetPanelAs<UI::Panels::AView>("Scene View").UnBind();
        uiManager->Render();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        device->PollEvents();
        window->SwapBuffers();
        inputManager->ClearEvents();
        clock.Update();
        if (objectPropChanged)
            GetScene()->RebuildInstances();
    }
    //回收资源
    Ret();
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