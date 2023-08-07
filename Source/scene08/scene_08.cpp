#include "scene_08.h"
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
        windowSettings.title = "Move Particle With Trail";
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
        scene::Scene* S = new scene::Scene08("Move Particle");
        S->Init();
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
            glClearColor(0., 0., 0., 0.);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);
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
            inputManager->ClearEvents();
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
namespace scene {
using namespace core;
using namespace asset;
using namespace component;
using namespace utils;
    Scene* instance = nullptr;
    float aspect=16.0f/9.0f;
    static int fps = 60.0f;
    static float interval = 0;
    static float total_time = 0.0f;
    static int frame_cnt = 0;
    struct FrameBuffer{
        int width;
        int height;
        float texelSizeX;
        float texelSizeY;
        GLuint tid = 0;
        GLuint id = 0;
        int attach(int id) {
            glBindTextureUnit(id, tid);
            return id;
        }
        void Bind() {
            glBindFramebuffer(GL_FRAMEBUFFER, id);
        }
        void UnBind() {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        FrameBuffer(int w, int h,uint internalFormat,uint param,uint format) {
            glGenFramebuffers(1, &id);
            glBindFramebuffer(GL_FRAMEBUFFER, id);
            glGenTextures(1, &tid);
            glBindTexture(GL_TEXTURE_2D, tid);
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_HALF_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, param);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, param);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tid, 0);
            auto status=glCheckNamedFramebufferStatus(id, GL_FRAMEBUFFER);
            width = w;
            height = h;
            texelSizeX = 1.0 / width;
            texelSizeY = 1.0 / height;
        }
        ~FrameBuffer() {
            glDeleteFramebuffers(1,&id);
            glDeleteTextures(1,&tid);


        }
    };
    struct FrameBufferDouble {
        std::shared_ptr<FrameBuffer> read;
        std::shared_ptr<FrameBuffer> write;
        FrameBufferDouble(int w, int h,uint internalFormat, uint param, uint format) {
            read = std::make_shared<FrameBuffer>(w,h, internalFormat,param,format);
            write = std::make_shared<FrameBuffer>(w, h,internalFormat,param,format);
        }
        void swap() {
            std::swap(read,write);
        }
    };
    //缓冲
    static asset_tmp<UBO> renderer_input = nullptr;

    std::shared_ptr<FrameBufferDouble>particle;//this use to record the info of every particle
    std::shared_ptr<FrameBufferDouble>coloroutput;//this use to output the result
    //着色计算
    std::shared_ptr<Shader>particleMoveProgram;
    std::shared_ptr<Shader>outPutProgram;//this use to print the color of a particle to a texture

    std::shared_ptr<Shader>displayMaterial;


    Scene08::~Scene08()
    {

        particle.reset();
        particleMoveProgram.reset();
        coloroutput.reset();
        outPutProgram.reset();
        displayMaterial.reset();

    }

    inline float random_double() {
        return rand() / (RAND_MAX + 1.0);
    }
    glm::vec3 HSVtoRGB(float h, float s, float v) {
        float r, g, b,  f, p, q, t;
        int i =h*6+0.5;
        f = h * 6 - i;
        p = v * (1 - s);
        q = v * (1 - f * s);
        t = v * (1 - (1 - f) * s);

        switch (i % 6) {
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
        }
        return {
            r,
            g,
            b
        };
    }
    glm::vec3 generateColor() {
        glm::vec3 c = HSVtoRGB(random_double(), 1.0, 1.0);
        c.r *= 0.15;
        c.g *= 0.15;
        c.b *= 0.15;
        return c;
    }
    void blit( FrameBuffer*f,bool clear=false) {
        if (f == nullptr) {
            glViewport(0,0,instance->width,instance->height);
        }
        else {
            f->Bind();
            glViewport(0,0,f->width,f->height);
            
        }
        if (clear) {
            glClearColor(0.0,0.0,0.0,1.0);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        Mesh::DrawQuad();
        if (f) {
            f->UnBind();
        }
    }
    void render( FrameBuffer* f=nullptr) {
        displayMaterial->Bind();
        coloroutput->read->attach(0);
        blit(f);

    }


    // this is called before the first frame, use this function to initialize your scene
    void Scene08::Init() {
        instance = this;
        Renderer::SetScene(this);
        this->title = "Particle";
        particleMoveProgram= MakeAsset<Shader>("res\\shaders\\particleMoveShader.glsl");
        displayMaterial = MakeAsset<Shader>("res\\shaders\\displayShaderSource.glsl");
        outPutProgram= MakeAsset<Shader>("res\\shaders\\particleShader.glsl");
        particle = std::make_shared<FrameBufferDouble>(this->width,this->height, GL_RGBA16F, GL_LINEAR, GL_RGBA);
        coloroutput = std::make_shared<FrameBufferDouble>(this->width, this->height, GL_RGBA16F, GL_LINEAR, GL_RGBA);
        Renderer::FaceCulling(true);
        Renderer::AlphaBlend(false);
        Renderer::SeamlessCubemap(true);

        // create the renderer input UBO on the first run (internal UBO)
        if (renderer_input == nullptr) {
            const std::vector<GLuint> offset{ 0U, 8U, 12U, 16U, 24U,32U};
            const std::vector<GLuint> length{ 8U, 4U, 4U, 4U, 8U,4U};
            const std::vector<GLuint> stride{ 8U, 4U, 4U, 8U, 8U,4U};
            renderer_input = WrapAsset<UBO>(11, offset, length, stride);
        }
       
        
    }

    void Scene08::UpdateScene(float dt)
    {

        frame_cnt++;
        total_time += dt;

        int resolution[2] = { width, height };
        renderer_input->SetUniform(0U, resolution); 
        renderer_input->SetUniform(1U, &total_time);
        renderer_input->SetUniform(2U, &dt);

        renderer_input->SetUniform(5U,(void*)&frame_cnt);
 
    }

    void Scene08::OnSceneRender(float dt) {
        particleMoveProgram->Bind();
        particle->read->attach(0);
        blit(particle->write.get());
        particle->swap();

        outPutProgram->Bind();
        coloroutput->read->attach(0);
        particle->read->attach(1);
        blit(coloroutput->write.get());
        coloroutput->swap();

    }

    void Scene08::OnImGuiRender(float dt) {   
        ImGui::Text("res:(%d,%d)",width,height);
        auto it=ImGui::GetMousePos();
       
        ImGui::Text("mouse:(%f,%f)",it.x,it.y);
        
    }
    void Scene08::Resize(int w, int h)
    {
      //由于Resize之前的信息丢失了
        Scene::Resize(w,h);
        aspect = (float)w / h;
        total_time = 0.0f;
        frame_cnt = 0;

        particle.reset();
        particle = std::make_shared<FrameBufferDouble>(w, h, GL_RGBA16F, GL_LINEAR, GL_RGBA);
        coloroutput = std::make_shared<FrameBufferDouble>(w, h, GL_RGBA16F, GL_LINEAR, GL_RGBA);
    }
    void Scene08::Present()
    {
        render();
    }
}




