#pragma once
#include<string>
#include"Scene.h"
#include"LoadScene.h"
#include"Renderer.h"
using namespace std;
namespace PathTrace{

    extern int sampleSceneIdx;
    extern std::vector<string> sceneFiles;
    extern int envMapIdx;
    extern std::vector<string> envMaps;
    extern int selectedInstance;
    extern float CameraMoveSpeed;
    extern bool objectPropChanged;
    extern bool showTransform;
    extern bool space_down;
    extern float screenX[2];
    extern float screenY[2] ;

    Scene* GetScene();
    Renderer* GetRenderer();
    void GetSceneFiles();

    RenderOptions& GetRenderOptions();
    void GetEnvMaps();

    void LoadScene(std::string sceneName);
 
    bool InitRenderer();

    void Ret();


}