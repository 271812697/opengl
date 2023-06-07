#pragma once
#include"openglApp.h"
#include"Opengl/scene/scene.h"

class GameApp:public openglApp
{
public:
	GameApp();
	
	std::shared_ptr<scene::Scene> sc;
	void attachScene(scene::Scene* s);
    bool Init();                         // 该父类方法需要初始化窗口和Direct3D部分			   
    void UpdateScene(float dt);   // 子类需要实现该方法，完成每一帧的更新
	void DrawScene();
	void OnResize(GLFWwindow* window, int width, int height);// 该父类方法需要在窗口大小变动的时候调用
};

