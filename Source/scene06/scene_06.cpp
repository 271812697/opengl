
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
#include "scene_06.h"
#include "../openglApp.h"

using namespace core;
using namespace asset;
using namespace component;
using namespace utils;

namespace scene {
    openglApp* instance =nullptr;
  

    struct FrameBuffer{ 
        std::shared_ptr<FBO> fbo;
        int width;
        int height;
        float texelSizeX;
        float texelSizeY;
        int attach(int id) {
            fbo->GetColorTexture(0).Bind(id);
            return id;
        }
        FrameBuffer(int w, int h) {
            width = w;
            height = h;
            texelSizeX = 1.0 / width;
            texelSizeY = 1.0 / height;
            fbo = std::make_shared<FBO>(w,h);
            fbo->AddColorTexture(1);
        }
    };
    struct FrameBufferDouble {
        std::shared_ptr<FrameBuffer> read;
        std::shared_ptr<FrameBuffer> write;
        FrameBufferDouble(int w, int h) {
            read = std::make_shared<FrameBuffer>(w,h);
            write = std::make_shared<FrameBuffer>(w, h);
        }
        void swap() {
            std::swap(read,write);
        }
    };
    std::shared_ptr<FrameBufferDouble>velocity;
    std::shared_ptr<FrameBufferDouble>dye;
    std::shared_ptr<FrameBuffer>divergence;
    std::shared_ptr<FrameBuffer>curl;
    std::shared_ptr<FrameBufferDouble>pressure;

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
        int SIM_RESOLUTION = 256;
        int DYE_RESOLUTION = 1024;
        float SPLAT_RADIUS = 0.25;
        float CURL = 30.0f;
        float PRESSURE = 0.8f;
        int PRESSURE_ITERATIONS = 20;
        float VELOCITY_DISSIPATION = 4;
        float DENSITY_DISSIPATION = 4;
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
        // Returns a random real in [0,1).
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

    void blit(const FrameBuffer*f,bool clear=false) {
        if (f == nullptr) {
            auto  Window = instance->GetWindowSize();
            glViewport(0,0,Window.first,Window.second);
        }
        else {
            glViewport(0,0,f->width,f->height);
            f->fbo->Bind();
        }
        if (clear) {
            glClearColor(0.0,0.0,0.0,1.0);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        Mesh::DrawQuad();
        if (f) {
            f->fbo->Unbind();
        }
    }
    void render(const FrameBuffer* f=nullptr) {
        displayMaterial->Bind();
        dye->read->attach(0);
        //velocity->read->attach(0);
        blit(f);

    }
    void splat(float x,float y,float dx,float dy,glm::vec3 color) {
        splatProgram->Bind();
        velocity->read->attach(0);
        splatProgram->SetUniform(1,16.0f/9);
        splatProgram->SetUniform(2, glm::vec3(dx, dy,0.0f));
        splatProgram->SetUniform(3, glm::vec2(x, y));
        splatProgram->SetUniform(4, 16.0f/9.0f*Config.SPLAT_RADIUS/100.0f);
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
        //由前一时刻的速度来更新这一刻的速度

        
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
        instance = GetOpenglApp();
        Renderer::SetScene(this);
        this->title = "Tiled Forward Renderer";


      
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


       openglApp* instance= GetOpenglApp();
       auto  Window = instance->GetWindowSize();

        //velocity
        velocity = std::make_shared<FrameBufferDouble>(16.0f / 9.0 * Config.SIM_RESOLUTION, Config.SIM_RESOLUTION);
        dye = std::make_shared<FrameBufferDouble>(16.0f / 9.0 * Config.DYE_RESOLUTION, Config.DYE_RESOLUTION);
        divergence = std::make_shared<FrameBuffer>(16.0f / 9.0 * Config.SIM_RESOLUTION, Config.SIM_RESOLUTION);
        curl = std::make_shared<FrameBuffer>(16.0f / 9.0 * Config.SIM_RESOLUTION, Config.SIM_RESOLUTION);
        pressure = std::make_shared<FrameBufferDouble>(16.0f / 9.0 * Config.SIM_RESOLUTION, Config.SIM_RESOLUTION);

        Renderer::FaceCulling(true);
        Renderer::AlphaBlend(false);
        Renderer::SeamlessCubemap(true);
        multipleSplats(15);
    }

    // this is called every frame, update your scene here and submit entities to the renderer
    void Scene06::OnSceneRender() {

        Renderer::Clear();
        render();


    }

    // this is called every frame, update your ImGui widgets here to control entities in the scene
    void Scene06::OnImGuiRender() {
        float dt = instance->GetDeltaTime();
        update(dt);
        ImVec2 mousePos = ImGui::GetMousePos();
        auto wsize = instance->GetWindowSize();
        float x = mousePos.x / wsize.first;
        float y = mousePos.y / wsize.second;
        y = 1 - y;        
        if (!pointer.down&&ImGui::IsMouseDown(0)) {
                pointer.down=true;
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
                pointer.deltaX = 16.0f/9.0f * (pointer.texcoordX - pointer.prevTexcoordX);
                pointer.deltaY = 16.0f/9.0f * (pointer.texcoordY - pointer.prevTexcoordY);
                pointer.move =abs(pointer.deltaX) > 0 || abs(pointer.deltaY) > 0;
        }
        if (ImGui::IsMouseReleased(0)) {
                pointer.down = false;
        }
        if (pointer.move) {
            pointer.move = false;
            splatPointer(pointer);
        }
        ImGui::Begin("Settings");
        ImGui::DragFloat("density diffusion",&Config.DENSITY_DISSIPATION,0,4);
        ImGui::DragFloat("velocity diffusion", &Config.VELOCITY_DISSIPATION, 0, 4);
        ImGui::DragFloat("pressure", &Config.PRESSURE, 0, 1);
        ImGui::DragFloat("vorticity", &Config.CURL, 0, 50);
        ImGui::DragFloat("splat radius", &Config.SPLAT_RADIUS, 0.01, 1);


        ImGui::End();
    }
}

#include"../all.h"

MAINSCENE(scene::Scene06,"01")