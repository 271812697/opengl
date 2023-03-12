#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include<string>
struct App {
    std::string gl_vendor, gl_renderer, gl_version, glsl_version;
    GLint gl_texsize, gl_texsize_3d, gl_texsize_cubemap;
    GLint gl_max_texture_units, gl_max_image_units;
    GLint gl_max_color_buffs;
    GLint gl_maxv_atcs, gl_maxf_atcs, gl_maxc_atcs;
    GLint gl_maxv_ubos, gl_maxg_ubos, gl_maxf_ubos, gl_maxc_ubos;
    GLint gl_maxf_ssbos, gl_maxc_ssbos;
    GLint cs_nx, cs_ny, cs_nz, cs_sx, cs_sy, cs_sz, cs_max_invocations;
};
class  openglApp;
App& GetAppInstance();
openglApp* GetOpenglApp();
class openglApp
{
public:
	openglApp();
	float     AspectRatio()const;
	int Run();                                // 运行程序，进行游戏主循环									  // 框架方法。客户派生类需要重载这些方法以实现特定的应用需求
	virtual bool Init();                      // 该父类方法需要初始化窗口和Direct3D部分
	               // 该父类方法需要在窗口大小变动的时候调用
	virtual void UpdateScene(float dt) = 0;   // 子类需要实现该方法，完成每一帧的更新
	virtual void DrawScene() = 0;
	virtual void OnResize(GLFWwindow* window, int width, int height);
    std::pair<int,int> GetWindowSize() {
        return std::make_pair(m_ClientWidth, m_ClientHeight);
    }

protected:
	bool InitMainWindow();
	bool InitOpengl();
	void CalculateFrameStats();
protected:
	GLFWwindow* window;
	// 派生类应该在构造函数设置好这些自定义的初始参数
	std::wstring m_MainWndCaption;                       // 主窗口标题
	int m_ClientWidth;                                   // 视口宽度
	int m_ClientHeight;
};

