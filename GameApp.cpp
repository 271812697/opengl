#define CP2
#ifdef CP1
#endif // C1
#ifdef CP2
#include "GameApp.h"
#include"core/log.h"
#include"core/sync.h"
#include<type_traits>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>

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

