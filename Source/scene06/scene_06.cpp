#include "scene_06.h"
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
        windowSettings.title = "Fluid Simulation";
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
        scene::Scene* S = new scene::Scene06("Fluid");
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
    std::shared_ptr<FrameBufferDouble>velocity;
    std::shared_ptr<FrameBufferDouble>dye;
    std::shared_ptr<FrameBuffer>divergence;
    std::shared_ptr<FrameBuffer>curl;
    std::shared_ptr<FrameBufferDouble>pressure;
    //着色计算
    std::shared_ptr<Shader>copyProgram;
    std::shared_ptr<Shader>clearProgram;
    std::shared_ptr<Shader>fulscreenProgram;
    std::shared_ptr<Shader>splatProgram;
    std::shared_ptr<Shader>advectionProgram;
    std::shared_ptr<Shader>divergenceProgram;
    std::shared_ptr<Shader>curlProgram;
    std::shared_ptr<Shader>vorticityProgram;
    std::shared_ptr<Shader>pressureProgram;
    std::shared_ptr<Shader>gradienSubtractProgram;
    std::shared_ptr<Shader>displayMaterial;

    Scene06::~Scene06()
    {
        displayMaterial.reset();
        gradienSubtractProgram.reset();
        pressureProgram.reset();
        vorticityProgram.reset();
        curlProgram.reset();
        divergenceProgram.reset();
        advectionProgram.reset();
        splatProgram.reset();
        fulscreenProgram.reset();
        clearProgram.reset();
        copyProgram.reset();
        pressure.reset();
        curl.reset();
        divergence.reset();
        velocity.reset();
        dye.reset();
    }
    struct {
        int SIM_RESOLUTION =256;
        int DYE_RESOLUTION = 1024;
        float SPLAT_RADIUS = 0.25;
        float CURL = 30.0f;
        float PRESSURE = 0.8f;
        int PRESSURE_ITERATIONS = 20;
        float VELOCITY_DISSIPATION = 4;
        float DENSITY_DISSIPATION = 0;
        float SPLAT_FORCE = 6000;

    }Config;
    struct Pointer{
        float texcoordX = 0;
        float texcoordY = 0;
        float prevTexcoordX = 0;
        float prevTexcoordY = 0;
        float deltaX = 0;
        float deltaY = 0;
        bool down = false;
        bool move = false;
        glm::vec3 color = { 30.0f,0.0f,300.0f };
    }pointer;
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
        dye->read->attach(0);
        blit(f);
    }
    void splat(float x,float y,float dx,float dy,glm::vec3 color) {
        splatProgram->Bind();
        velocity->read->attach(0);
        splatProgram->SetUniform(1,aspect);
        splatProgram->SetUniform(2, glm::vec3(dx, dy,0.0f));
        splatProgram->SetUniform(3, glm::vec2(x, y));
        splatProgram->SetUniform(4, aspect*Config.SPLAT_RADIUS/100.0f);
        blit(velocity->write.get());
        velocity->swap();

        dye->read->attach(0);
        splatProgram->SetUniform(2,color);
        blit(dye->write.get());
        dye->swap();

    }
    void splatPointer(Pointer pointer) {
        float dx = pointer.deltaX * Config.SPLAT_FORCE;
        float dy = pointer.deltaY * Config.SPLAT_FORCE;
        splat(pointer.texcoordX,pointer.texcoordY,dx,dy,pointer.color);
    }
    void multipleSplats(int amount) {
        for (int i = 0; i < amount; i++) {
            auto color = generateColor();
            color.r *= 10.0;
            color.g *= 10.0;
            color.b *= 10.0;
            float x = random_double();
            float y = random_double();
            float dx = 1000 * (random_double() - 0.5);
            float dy = 1000 * (random_double() - 0.5);
            splat(x,y,dx,dy,color);
        }
    }
    void update(float dt) {

        //根据速度来算旋度
        curlProgram->Bind();
        curlProgram->SetUniform(3,glm::vec2(velocity->read->texelSizeX,velocity->read->texelSizeY));
        velocity->read->attach(0);
        blit(curl.get());
        //由旋度算力，进一步算速度
        vorticityProgram->Bind();
        vorticityProgram->SetUniform(3, glm::vec2(velocity->read->texelSizeX, velocity->read->texelSizeY));
        velocity->read->attach(0);
        curl->attach(1);
        vorticityProgram->SetUniform(4,Config.CURL);
        vorticityProgram->SetUniform(5, dt);
        blit(velocity->write.get());
        velocity->swap();
        //由速度算散度
        divergenceProgram->Bind();
        divergenceProgram->SetUniform(3, glm::vec2(velocity->read->texelSizeX, velocity->read->texelSizeY));
        velocity->read->attach(0);
        blit(divergence.get());
        //衰减压力
        clearProgram->Bind();
        pressure->read->attach(0);
        clearProgram->SetUniform(1,Config.PRESSURE );
        blit(pressure->write.get());
        pressure->swap();
        //由散度来更新压力
        pressureProgram->Bind();
        pressureProgram->SetUniform(3, glm::vec2(velocity->read->texelSizeX, velocity->read->texelSizeY));
        divergence->attach(0);
        for (int i = 0; i < Config.PRESSURE_ITERATIONS; i++) {
            pressure->read->attach(1);
            blit(pressure->write.get());
            pressure->swap();
        }
        //由压力来算速度
        gradienSubtractProgram->Bind();
        gradienSubtractProgram->SetUniform(3, glm::vec2(velocity->read->texelSizeX, velocity->read->texelSizeY));
        pressure->read->attach(0);
        velocity->read->attach(1);
        blit(velocity->write.get());
        velocity->swap();
        //对流由前一时刻的速度来更新这一刻的速度


        advectionProgram->Bind();
        advectionProgram->SetUniform(3, glm::vec2(velocity->read->texelSizeX, velocity->read->texelSizeY));
        advectionProgram->SetUniform(7,1);
        velocity->read->attach(0);
        advectionProgram->SetUniform(5,dt);
        advectionProgram->SetUniform(6, Config.VELOCITY_DISSIPATION);
        blit(velocity->write.get());
        velocity->swap();

        velocity->read->attach(0);
        dye->read->attach(1);
        advectionProgram->SetUniform(7, 0);
        advectionProgram->SetUniform(6, Config.DENSITY_DISSIPATION);
        blit(dye->write.get());
        dye->swap();
    }
    // this is called before the first frame, use this function to initialize your scene
    void Scene06::Init() {
        instance = this;
        Renderer::SetScene(this);
        this->title = "Fluid";
        //初始化
        fulscreenProgram = MakeAsset<Shader>("res\\shaders\\fullscreen.glsl");
        copyProgram= MakeAsset<Shader>("res\\shaders\\copyShader.glsl");
        clearProgram = MakeAsset<Shader>("res\\shaders\\clearShader.glsl");
        splatProgram = MakeAsset<Shader>("res\\shaders\\splatShader.glsl");
        displayMaterial = MakeAsset<Shader>("res\\shaders\\displayShaderSource.glsl");
        curlProgram = MakeAsset<Shader>("res\\shaders\\curlShader.glsl");
        vorticityProgram = MakeAsset<Shader>("res\\shaders\\vorticityShader.glsl");
        divergenceProgram = MakeAsset<Shader>("res\\shaders\\divergenceShader.glsl");
        pressureProgram= MakeAsset<Shader>("res\\shaders\\pressureShader.glsl");
        gradienSubtractProgram = MakeAsset<Shader>("res\\shaders\\gradientSubtractShader.glsl");
        advectionProgram = MakeAsset<Shader>("res\\shaders\\advectionShader.glsl");

        //velocity
        velocity = std::make_shared<FrameBufferDouble>(aspect * Config.SIM_RESOLUTION, Config.SIM_RESOLUTION,GL_RG16F,GL_LINEAR,GL_RG);
        dye = std::make_shared<FrameBufferDouble>(aspect * Config.DYE_RESOLUTION, Config.DYE_RESOLUTION,GL_RGBA16F, GL_LINEAR,GL_RGBA);
        divergence = std::make_shared<FrameBuffer>(aspect * Config.SIM_RESOLUTION, Config.SIM_RESOLUTION, GL_R16F, GL_NEAREST, GL_RED);
        curl = std::make_shared<FrameBuffer>(aspect * Config.SIM_RESOLUTION, Config.SIM_RESOLUTION, GL_R16F, GL_NEAREST, GL_RED);
        pressure = std::make_shared<FrameBufferDouble>(aspect * Config.SIM_RESOLUTION, Config.SIM_RESOLUTION, GL_R16F, GL_NEAREST, GL_RED);

        Renderer::FaceCulling(true);
        Renderer::AlphaBlend(false);
        Renderer::SeamlessCubemap(true);
        multipleSplats(15);
    }

    void Scene06::UpdateScene(float dt)
    {
        interval += dt;
        if (interval >= 1.0f / fps) {
            update(interval);
            interval = 0;
        }
    }

    void Scene06::OnSceneRender(float dt) {

    }

    void Scene06::OnImGuiRender(float dt) {   
        if (ImGui::IsKeyPressed(ImGuiKey_::ImGuiKey_Space)) {
            multipleSplats(15);
        }
        auto& view = m_panelsManager->GetPanelAs<UI::Panels::AView>("Scene View");
        if (ImGui::IsKeyDown(ImGuiKey_::ImGuiKey_W)&&view.IsHovered()) {
            ImVec2 mousePos = ImGui::GetMousePos();
            auto[xx,yy]= view.GetPosition();
            auto [sizew, sizeh] = view.GetSize();
            float x =( mousePos.x-xx) / sizew;
            float y = (mousePos.y -yy-25)/ sizeh;
            y = 1 - y;
            
            if (!pointer.down && ImGui::IsMouseDown(0)) {
                pointer.down = true;
                pointer.move = false;
                pointer.texcoordX = x;
                pointer.texcoordY = y;
                pointer.prevTexcoordX = pointer.texcoordX;
                pointer.prevTexcoordY = pointer.texcoordY;
                pointer.deltaX = 0;
                pointer.deltaY = 0;
                pointer.color = generateColor();
            }
            if (pointer.down) {
                pointer.prevTexcoordX = pointer.texcoordX;
                pointer.prevTexcoordY = pointer.texcoordY;
                pointer.texcoordX = x;
                pointer.texcoordY = y;
                pointer.deltaX = aspect * (pointer.texcoordX - pointer.prevTexcoordX);
                pointer.deltaY = aspect * (pointer.texcoordY - pointer.prevTexcoordY);
                pointer.move = abs(pointer.deltaX) > 0 || abs(pointer.deltaY) > 0;
            }
            if (ImGui::IsMouseReleased(0)) {
                pointer.down = false;
            }
            if (pointer.move) {
                pointer.move = false;
                splatPointer(pointer);
            }
        }
        static int cur = 1;
        const char* quality[3] = {"low 128","meidum 256","high 1024"};
        int res[3] = {128,256,1024};
        ImGui::SliderInt("fps", &fps, 60, 1000);
        if (ImGui::Combo("Render Quality", &cur, quality, 3)) {
            velocity.reset();
            divergence.reset();
            curl.reset();
            pressure.reset();
            velocity = std::make_shared<FrameBufferDouble>(aspect * res[cur], res[cur], GL_RG16F, GL_LINEAR, GL_RG);
            divergence = std::make_shared<FrameBuffer>(aspect * res[cur], res[cur], GL_R16F, GL_NEAREST, GL_RED);
            curl = std::make_shared<FrameBuffer>(aspect * res[cur], res[cur], GL_R16F, GL_NEAREST, GL_RED);
            pressure = std::make_shared<FrameBufferDouble>(aspect * res[cur], res[cur], GL_R16F, GL_NEAREST, GL_RED);

        }
        ImGui::SliderFloat("density diffusion",&Config.DENSITY_DISSIPATION,0,4);
        ImGui::SliderFloat("velocity diffusion", &Config.VELOCITY_DISSIPATION, 0, 4);
        ImGui::SliderFloat("pressure", &Config.PRESSURE, 0, 1);
        ImGui::SliderFloat("vorticity", &Config.CURL, 0, 50);
        ImGui::SliderFloat("splat radius", &Config.SPLAT_RADIUS, 0.01, 1);
        ImGui::Image((void*)velocity->read->tid, ImVec2(aspect * 100, 100), ImVec2(0.f, 1.f), ImVec2(1.f, 0.f));
        ImGui::SameLine();ImGui::Text("velocity");
        ImGui::Image((void*)divergence->tid, ImVec2(aspect * 100, 100), ImVec2(0.f, 1.f), ImVec2(1.f, 0.f));
        ImGui::SameLine();ImGui::Text("divergence");
        ImGui::Image((void*)curl->tid, ImVec2(aspect * 100, 100), ImVec2(0.f, 1.f), ImVec2(1.f, 0.f));
        ImGui::SameLine();ImGui::Text("curl");
        ImGui::Image((void*)pressure->read->tid, ImVec2(aspect * 100, 100), ImVec2(0.f, 1.f), ImVec2(1.f, 0.f));
        ImGui::SameLine();ImGui::Text("pressure");
    }
    void Scene06::Resize(int w, int h)
    {
      //由于Resize之前的信息丢失了
        Scene::Resize(w,h);
        aspect = (float)w / h;
        velocity.reset();
        divergence.reset();
        curl.reset();
        pressure.reset();
        dye.reset();
        velocity = std::make_shared<FrameBufferDouble>(aspect * Config.SIM_RESOLUTION, Config.SIM_RESOLUTION, GL_RG16F, GL_LINEAR, GL_RG);
        dye = std::make_shared<FrameBufferDouble>(aspect * Config.DYE_RESOLUTION, Config.DYE_RESOLUTION, GL_RGBA16F, GL_LINEAR, GL_RGBA);
        divergence = std::make_shared<FrameBuffer>(aspect * Config.SIM_RESOLUTION, Config.SIM_RESOLUTION, GL_R16F, GL_NEAREST, GL_RED);
        curl = std::make_shared<FrameBuffer>(aspect * Config.SIM_RESOLUTION, Config.SIM_RESOLUTION, GL_R16F, GL_NEAREST, GL_RED);
        pressure = std::make_shared<FrameBufferDouble>(aspect * Config.SIM_RESOLUTION, Config.SIM_RESOLUTION, GL_R16F, GL_NEAREST, GL_RED);   
    }
    void Scene06::Present()
    {
        render();
    }
}




