#define CP2
#ifdef CP1
#endif // C1
#ifdef CP2
#include "GameApp.h"

using namespace std;


GameApp::GameApp() :openglApp()
{

}

void GameApp::attachScene(scene::Scene* s)
{
    sc.reset(s);
}

bool GameApp::Init()
{
    openglApp::Init();
    sc->Init();
    return true;
}
void GameApp::UpdateScene(float dt)
{
    this->dt = dt;
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

