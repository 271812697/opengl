#define CP2
#ifdef CP1

#include "GameApp.h"
#include"Shader.h"
#include"Geometry.h"
#include"core/log.h"
#include"core/sync.h"
#include<type_traits>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include"Quaternion.h"
#include"VaoObject.h"
#include"Node.h"
#include"GameObject.h"
#include"pch.h"
#include"asset/all.h"
#include"ecs/entt.hpp"
#include"./component/all.h"
using namespace std;
template<typename T,typename ...Args>
std::shared_ptr<T> MakeRef(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}
using ID = size_t;
ID StringToHash(std::string_view str) {
    static std::hash<std::string_view> h;
    return  h(str);     
}
class ResourceManager {
private:
    std::map<ID, std::type_index> registry;
    std::map<ID, asset_ref<void>> resources;

public:
    ResourceManager() {}
    ~ResourceManager() {}

    template<typename T>
    void Add(ID key, const asset_ref<T>& resource) {
        if (registry.find(key) != registry.end()) {
            CORE_ERROR("Duplicate key already exists, cannot add the resource...");
            return;
        }
        registry.try_emplace(key,typeid(T));
        resources.insert_or_assign(key,std::static_pointer_cast<void>(resource));
    }

    template<typename T>
    asset_ref<T> Get(ID key) const {
        if (registry.find(key) == registry.end()) {
            CORE_ERROR("Invalid resource key!");
            return nullptr;
        }
        else if (registry.at(key)!=std::type_index(typeid(T))) {
            CORE_ERROR("Mismatched resource type");
            return nullptr;
        }
        return std::static_pointer_cast<T>(resources.at(key));
    }

    void Del(ID key) {
        if (registry.find(key) != registry.end()) {
            registry.erase(key);
            resources.erase(key);
        }

    }
    void Clear() {
        registry.clear();
        resources.clear();

    }
};
ResourceManager resource_manager;
std::map<std::string, unsigned int>index_count;

entt::registry registry;
std::unordered_map<ID, entt::entity>entity_table;
Opengl::Node a, b, c;
Opengl::GameObject boxa, boxb, boxc;
struct FirstCamera {
    Opengl::Node Camera;
    GLFWwindow* window;
    void setTransform(Opengl::Vector3 p, Opengl::Vector3 d) {
        if (window == nullptr)
            exit(0);
        Camera.SetDirection(-d);
        Camera.SetPosition(p);
        Camera.SetScale({ 1,1,1 });
    }
    Opengl::Matrix4 getView() {
        return  Camera.GetWorldTransform().Inverse().ToMatrix4().Transpose();
    }
    void update(float dt) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            Camera.Translate(-Camera.GetDirection() * dt * 3.0, Opengl::TS_PARENT);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            Camera.Translate(Camera.GetDirection() * dt * 3.0, Opengl::TS_PARENT);
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            Camera.Translate(-Camera.GetRight() * dt * 3.0, Opengl::TS_PARENT);
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            Camera.Translate(Camera.GetRight() * dt * 3.0, Opengl::TS_PARENT);
        }

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        {
            Camera.Translate(Camera.GetUp() * dt * 3.0, Opengl::TS_PARENT);
        }
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        {
            Camera.Translate(-Camera.GetUp() * dt * 3.0, Opengl::TS_PARENT);
        }
        static double x = -1, y = -1;
        static double prex = -1, prey = -1;
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {

            glfwGetCursorPos(window, &x, &y);
            if (prex >= 0) {
                Camera.Pitch(-(y - prey) * 5 * dt, Opengl::TS_LOCAL);
                Camera.Yaw(-(x - prex) * 15 * dt, Opengl::TS_LOCAL);

            }
            prex = x;
            prey = y;
        }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
            prex = prey = -1;
        }
    }
}Camera;

GameApp::GameApp() :openglApp()
{

}
void PrecomputeIBL() {
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    auto irradiance_shader =asset::CShader("irradiance_map.glsl");
    irradiance_shader.Inspect();
    auto prefilter_shader =asset::CShader("prefilter_envmap.glsl");
    prefilter_shader.Inspect();
    auto envBRDF_shader = asset::CShader("environment_BRDF.glsl");
    envBRDF_shader.Inspect();
    //用来做IBL预计算的立方体贴图
    auto env_map = MakeRef<asset::Texture>("F:/C++/sketchpad/res/texture/HDRI/cosmic/",".hdr",2048,0);
    env_map->Bind(0);
    //计算IBL的diffuse项
    auto irradiance_map = MakeRef<asset::Texture>(GL_TEXTURE_CUBE_MAP, 128, 128, 6, GL_RGBA16F, 1);
    //计算IBL的specular项
    auto prefiltered_map = MakeRef<asset::Texture>(GL_TEXTURE_CUBE_MAP, 2048, 2048, 6, GL_RGBA16F, 8);
    //计算BRDF
    auto BRDF_LUT = MakeRef<asset::Texture>(GL_TEXTURE_2D, 1024, 1024, 1, GL_RGBA16F, 1);
    CORE_INFO("Precomputing diffuse irradiance map from {0}", "F:/C++/sketchpad/res/texture/HDRI/cosmic/");
    irradiance_map->BindILS(0, 0, GL_WRITE_ONLY);

    if (irradiance_shader.Bind(); true) {
        irradiance_shader.Dispatch(128 / 32, 128 / 32, 6);
        irradiance_shader.SyncWait(GL_TEXTURE_FETCH_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);

        auto irradiance_fence = core::Sync(0);
        irradiance_fence.ClientWaitSync();
        irradiance_map->UnbindILS(0);
    }
    asset::Texture::Copy(*env_map,0,*prefiltered_map,0);
    const GLuint max_level = prefiltered_map->n_levels - 1;
    GLuint resolution = prefiltered_map->width / 2;
    prefilter_shader.Bind();
    CORE_INFO("Precomputing specular prefiltered envmap from {0}", "F:/C++/sketchpad/res/texture/HDRI/cosmic/");
    for (unsigned int level = 1; level <= max_level; level++, resolution /= 2) {
        float roughness = level / static_cast<float>(max_level);
        prefilter_shader.SetUniform(0, roughness);
        GLuint n_groups = glm::max<GLuint>(resolution/32,1);

        prefiltered_map->BindILS(level,1,GL_WRITE_ONLY);
        prefilter_shader.Dispatch(n_groups,n_groups,6);
        prefilter_shader.SyncWait(GL_TEXTURE_FETCH_BARRIER_BIT|GL_ATOMIC_COUNTER_BARRIER_BIT);

        auto prefilter_fence = core::Sync(level);
        prefilter_fence.ClientWaitSync();
        prefiltered_map->UnbindILS(1);
    }
    CORE_INFO("Precomputing specular environment BRDF from {0}", "F:/C++/sketchpad/res/texture/HDRI/cosmic/");
    BRDF_LUT->BindILS(0,2,GL_WRITE_ONLY);
    if (envBRDF_shader.Bind(); true) {
        envBRDF_shader.Dispatch(1024 / 32, 1024 / 32, 1);
        envBRDF_shader.SyncWait(GL_ALL_BARRIER_BITS);
        core::Sync::WaitFinish();
        BRDF_LUT->UnbindILS(2);
    }
    resource_manager.Add(StringToHash("env_map"), env_map);
    resource_manager.Add(StringToHash("irradiance_map"), irradiance_map);
    resource_manager.Add(StringToHash("prefiltered_map"), prefiltered_map);
    resource_manager.Add(StringToHash("BRDF_LUT"), BRDF_LUT);

}
void InitScene() {
    
    PrecomputeIBL();
    resource_manager.Add(StringToHash("environment_map"),MakeRef<asset::Texture>("E:/C++/LearnOpenGL/resources/textures/hdr/newport_loft.hdr",2048,0));
    resource_manager.Get<asset::Texture>(StringToHash("environment_map"))->Bind(0);
    resource_manager.Add(StringToHash("modelShader"), MakeRef<asset::Shader>("model.glsl"));
    resource_manager.Add(StringToHash("basicShader"), MakeRef<asset::Shader>("shaders/basic.glsl"));
    resource_manager.Get<asset::Shader>(StringToHash("basicShader"))->Inspect();
    resource_manager.Add(StringToHash("SphereMesh"), MakeRef<component::Mesh>(component::Primitive::Sphere));
    resource_manager.Add(StringToHash("TorusMesh"), MakeRef<component::Mesh>(component::Primitive::Torus));
    resource_manager.Add(StringToHash("TetrahedronMesh"), MakeRef<component::Mesh>(component::Primitive::Tetrahedron));
    resource_manager.Add(StringToHash("tex"), MakeRef<asset::Texture>("resources/textures/bricks2.jpg"));
    resource_manager.Add(StringToHash("Material"), MakeRef<component::Material>(resource_manager.Get<asset::Shader>(StringToHash("basicShader"))));
   
 
    entity_table[StringToHash("model")] = registry.create();
    auto model = entity_table[StringToHash("model")];
    registry.emplace<component::Model>(model, "C:/Users/271812697/Desktop/donghua/Zhaolinger/Hero_Zhaolinger/Hero_Zhaolinger.fbx",
        component::Quality::High, true);
    auto& it = registry.get<component::Model>(model);
    it.AttachMotion("C:/Users/271812697/Desktop/donghua/Zhaolinger/Hero_Zhaolinger/Hero_Zhaolinger.fbx");
    registry.emplace<component::Animator>(model,&it);

}

bool GameApp::Init()
{

    openglApp::Init();
    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(45.0f), this->AspectRatio(), 0.1f, 100.0f);
    a.SetPosition({ 0,1,0 });
    a.SetDirection({ 0,0,1 });
    a.SetScale({ 0.5,0.5,0.5 });
    b.SetParent(&a);
    b.SetPosition({ 0,6,0 });
    b.SetDirection({ 0,0,1 });
    b.SetScale({ 1,1,1 });
    c.SetParent(&b);
    c.SetPosition({ 0,2,0 });
    c.SetDirection({ 0,0,1 });
    c.SetScale({ 1,1,1 });

  

    boxa.TransForm = &a;
    boxb.TransForm = &b;
    boxc.TransForm = &c;

    Camera.window = this->window;
    Camera.setTransform({ 0,0,3 }, { 0,0,-1 });

    InitScene();

    resource_manager.Add(StringToHash("FBO"), MakeRef<asset::FBO>(m_ClientWidth, m_ClientHeight));
    resource_manager.Get<asset::FBO>(StringToHash("FBO"))->AddDepStTexture();
    resource_manager.Get<asset::FBO>(StringToHash("FBO"))->AddColorTexture(1);

    resource_manager.Get<asset::Texture>(StringToHash("tex"))->Bind(1);
    resource_manager.Get<asset::Shader>(StringToHash("basicShader"))->Bind();
    resource_manager.Get<asset::Shader>(StringToHash("basicShader"))->SetUniform(5, projection);
    resource_manager.Get<asset::Shader>(StringToHash("modelShader"))->SetUniform(5, projection);


    resource_manager.Add(StringToHash("material"),MakeRef<component::Material>(resource_manager.Get<asset::Shader>(StringToHash("modelShader"))));
    
   
    return true;

}
void GameApp::UpdateScene(float dt)
{

    Camera.update(dt);
    resource_manager.Get<asset::Shader>(StringToHash("basicShader"))->SetUniform(4, Camera.getView());
    resource_manager.Get<asset::Shader>(StringToHash("modelShader"))->SetUniform(4, Camera.getView());
    
    auto& animator = registry.get<component::Animator>(entity_table[StringToHash("model")]);
    static float t = 0.0f;
    
    
    if (t > 10.0f) {
        t = 0.0f;
        animator.Gonext();
    }
    animator.Update( dt);   
    glEnable(GL_DEPTH_TEST);
  
    glEnable(GL_STENCIL_TEST);
}
void GameApp::DrawScene()
{
    
    resource_manager.Get<asset::FBO>(StringToHash("FBO"))->Bind(); 
    //
    resource_manager.Get<asset::FBO>(StringToHash("FBO"))->Clear();
 
   

    
    resource_manager.Get<asset::Shader>(StringToHash("basicShader"))->Bind();
    boxa.TransForm->RotateAround({ 0,0,0 }, Opengl::Quaternion(1.0 / 144 * 18), Opengl::TS_PARENT);
    boxa.Draw(*resource_manager.Get<asset::Shader>(StringToHash("basicShader")));
    resource_manager.Get<component::Mesh>(StringToHash("SphereMesh"))->Draw();

    boxb.TransForm->RotateAround({ 0,0,0 }, Opengl::Quaternion(1.0 / 144 * 18), Opengl::TS_PARENT);
    boxb.Draw(*resource_manager.Get<asset::Shader>(StringToHash("basicShader")));
    resource_manager.Get<component::Mesh>(StringToHash("TorusMesh"))->Draw();

    boxc.TransForm->RotateAround({ 0,0,0 }, Opengl::Quaternion(1.0 / 144 * 18), Opengl::TS_PARENT);
    boxc.Draw(*resource_manager.Get<asset::Shader>(StringToHash("basicShader")));
    resource_manager.Get<component::Mesh>(StringToHash("TetrahedronMesh"))->Draw();


  
    resource_manager.Get<asset::Shader>(StringToHash("modelShader"))->Bind();
    auto& model = registry.get<component::Model>(entity_table[StringToHash("model")]);
    auto& animator = registry.get<component::Animator>(entity_table[StringToHash("model")]);
    
    resource_manager.Get<asset::Shader>(StringToHash("modelShader"))->SetUniformArray(100,
        animator.bone_transforms.size(), animator.bone_transforms);
    
    for (auto mesh:model.meshes) {
        mesh.Draw();
    }
    resource_manager.Get<asset::FBO>(StringToHash("FBO"))->Unbind();

    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClearDepth(1.0f);
    glClearStencil(0);  // 8-bit integer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    resource_manager.Get<asset::FBO>(StringToHash("FBO"))->Draw(0);
}
void GameApp::OnResize(GLFWwindow* window, int width, int height)
{
    openglApp::OnResize(window, width, height);
    resource_manager.Del(StringToHash("FBO"));
    resource_manager.Add(StringToHash("FBO"), MakeRef<asset::FBO>(m_ClientWidth, m_ClientHeight));
    resource_manager.Get<asset::FBO>(StringToHash("FBO"))->AddDepStTexture();
    resource_manager.Get<asset::FBO>(StringToHash("FBO"))->AddColorTexture(1);
    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(45.0f), AspectRatio(), 0.1f, 100.0f);
    resource_manager.Get<asset::Shader>(StringToHash("basicShader"))->SetUniform(5, projection);
    resource_manager.Get<asset::Shader>(StringToHash("modelShader"))->SetUniform(5, projection);
}

#endif // C1
#ifdef CP2
#include "GameApp.h"
#include"Shader.h"
#include"Geometry.h"
#include"core/log.h"
#include"core/sync.h"
#include<type_traits>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include"Quaternion.h"
#include"VaoObject.h"
#include"Node.h"
#include"GameObject.h"
#include"pch.h"
#include"scene/renderer.h"
#include"example/all.h"

using namespace std;
std::map<int, scene::Scene*>table;
int sindex = 3;
int lastindex = sindex;

void SwichScene() {
    ImGui::PushID("Choose Scene");
    ImGui::Begin("\xef\x80\x95" " Inspector", 0);
    if (ImGui::Combo("Scene", &sindex, str, scene_cnt)) {
        //detach
        {
            delete table[lastindex];
            table[lastindex] = nullptr;
            glFinish();
            scene::Renderer::Reset();
        }
        lastindex = sindex;
        //attach
        {

            table[sindex] = attach_Scene(sindex);
            table[sindex]->Init();
            glFinish();
        }
    }
    ImGui::End();
    ImGui::PopID();
}
GameApp::GameApp() :openglApp()
{

}

bool GameApp::Init()
{
    openglApp::Init();
   
    table[sindex] = attach_Scene(sindex);
    table[sindex]->Init();


    return true;
}
void GameApp::UpdateScene(float dt)
{
    SwichScene();
    table[sindex]->OnImGuiRender(); 
}
void GameApp::DrawScene()
{
    table[sindex]->OnSceneRender();
}
void GameApp::OnResize(GLFWwindow* window, int width, int height)
{
    openglApp::OnResize(window, width, height);
}
#endif // C2

