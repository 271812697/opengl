#pragma once
#include"openglApp.h"
#include"Opengl/scene/scene.h"

class GameApp:public openglApp
{
public:
	
	
	GameApp();
	void attachScene(scene::Scene* s);
    bool Init();                         // �ø��෽����Ҫ��ʼ�����ں�Direct3D����			   
    void UpdateScene(float dt);   // ������Ҫʵ�ָ÷��������ÿһ֡�ĸ���
	void DrawScene();
	void OnResize(GLFWwindow* window, int width, int height);// �ø��෽����Ҫ�ڴ��ڴ�С�䶯��ʱ�����
};

