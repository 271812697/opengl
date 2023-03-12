#define CP2
#ifdef CP1
#endif // C1
#ifdef CP2
#include "GameApp.h"

using namespace std;


GameApp::GameApp() :openglApp()
{

}
scene::Scene* sc = nullptr;
void GameApp::attachScene(scene::Scene* s)
{
    sc = s;
}

bool GameApp::Init()
{
    openglApp::Init();
   
    sc->Init();


    return true;
}
void GameApp::UpdateScene(float dt)
{
   
    sc->OnImGuiRender(); 
}
void GameApp::DrawScene()
{
    sc->OnSceneRender();
}
void GameApp::OnResize(GLFWwindow* window, int width, int height)
{
    openglApp::OnResize(window, width, height);
}
#endif // C2

