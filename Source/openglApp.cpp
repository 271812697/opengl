#include "openglApp.h"
#include<iostream>
#include <sstream>
#include"Opengl/core/log.h"
#include"Opengl/core/event.h"
#include"Opengl/core/window.h"
#include"Opengl/core/clock.h"
#include"Opengl/scene/ui.h"
openglApp* instance;
App app;
App& GetAppInstance() {
    return app;
}
openglApp* GetOpenglApp() {
    return instance;
}

openglApp::openglApp():m_ClientWidth(1600),m_ClientHeight(900)
{
    if (instance != nullptr)
        return;
    instance = this;
	
   
}
openglApp::~openglApp()
{
    ::core::Log::Shutdown();
}
float openglApp::AspectRatio() const
{
    return 1.0f*m_ClientWidth/m_ClientHeight;
}
int openglApp::Run()
{
  
    static float time = 0.0f;
    while (!glfwWindowShouldClose(window))
    {
       
        CalculateFrameStats();
        ::core::Clock::Update();
        ::scene::ui::NewFrame();
        float nowtime = glfwGetTime();
        UpdateScene(nowtime-time);
        time = nowtime;
        DrawScene();
        ::scene::ui::EndFrame();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    ::scene::ui::Clear();
    glfwTerminate();
    return 0;
}
bool openglApp::Init()
{
    ::core::Log::Init();
    InitMainWindow();
    
    CORE_INFO("Registering window event callbacks ...");
    ::core::Event::RegisterCallbacks();

    ::scene::ui::Init();
    return true;
}
void glfwSetFramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    instance->OnResize(window,width,height);
}
void openglApp::OnResize(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    m_ClientHeight = height;
    m_ClientWidth = width;
}
bool openglApp::InitMainWindow()
{
 
    
    ::core::Window::Init();
    this->window = ::core::Window::window_ptr;
    //output gl profile
   
    CORE_INFO(std::string(reinterpret_cast<const char*>(glGetString(GL_VENDOR))).c_str());
    CORE_INFO(std::string(reinterpret_cast<const char*>(glGetString(GL_RENDERER))).c_str());
    CORE_INFO(std::string(reinterpret_cast<const char*>(glGetString(GL_VERSION))).c_str());
    CORE_INFO(std::string(reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION))).c_str());
    // texture size limit, max number of texture units and image units
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &app.gl_texsize);
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &app.gl_texsize_3d);
    glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &app.gl_texsize_cubemap);
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &app.gl_max_texture_units);
    glGetIntegerv(GL_MAX_IMAGE_UNITS, &app.gl_max_image_units);

    // max number of atomic counters in each shader stage
    glGetIntegerv(GL_MAX_VERTEX_ATOMIC_COUNTERS, &app.gl_maxv_atcs);
    glGetIntegerv(GL_MAX_FRAGMENT_ATOMIC_COUNTERS, &app.gl_maxf_atcs);
    glGetIntegerv(GL_MAX_COMPUTE_ATOMIC_COUNTERS, &app.gl_maxc_atcs);

    // max number of uniform blocks in each shader stage
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &app.gl_maxv_ubos);
    glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_BLOCKS, &app.gl_maxg_ubos);
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &app.gl_maxf_ubos);
    glGetIntegerv(GL_MAX_COMPUTE_UNIFORM_BLOCKS, &app.gl_maxc_ubos);

    // max number of shader storage blocks in the fragment shader and compute shader
    glGetIntegerv(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, &app.gl_maxf_ssbos);
    glGetIntegerv(GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS, &app.gl_maxc_ssbos);

    // max number of work groups in the compute shader
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &app.cs_nx);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &app.cs_ny);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &app.cs_nz);

    // compute shader work groups size limit
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &app.cs_sx);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &app.cs_sy);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &app.cs_sz);

    // max number of threads in the compute shader
    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &app.cs_max_invocations);

    // max number of drawable color buffers in a user-defined framebuffer
    GLint max_color_attachments, max_draw_buffers;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &max_color_attachments);
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &max_draw_buffers);
    app.gl_max_color_buffs = std::min(max_color_attachments, max_draw_buffers); 
    std::cout << "  GPU Vendor Name:   " << app.gl_vendor << std::endl;
    std::cout << "  OpenGL Renderer:   " << app.gl_renderer << std::endl;
    std::cout << "  OpenGL Version:    " << app.gl_version << std::endl;
    std::cout << "  GLSL Core Version: " << app.glsl_version << '\n' << std::endl;

    std::cout << "$ Maximum supported texture size: " << '\n' << std::endl;
    std::cout << "  1D / 2D texture (width and height): " << app.gl_texsize << std::endl;
    std::cout << "  3D texture (width, height & depth): " << app.gl_texsize_3d << std::endl;
    std::cout << "  Cubemap texture (width and height): " << app.gl_texsize_cubemap << std::endl;
    std::cout << "  Max number of image units: " << app.gl_max_image_units << std::endl;
    std::cout << "  Max number of texture units: " << app.gl_max_texture_units << '\n' << std::endl;

    std::cout << "$ Maximum allowed number of atomic counters: " << '\n' << std::endl;
    std::cout << "  Vertex shader:   " << app.gl_maxv_atcs << std::endl;
    std::cout << "  Fragment shader: " << app.gl_maxf_atcs << std::endl;
    std::cout << "  Compute shader:  " << app.gl_maxc_atcs << '\n' << std::endl;

    std::cout << "$ Maximum allowed number of uniform buffers: " << '\n' << std::endl;
    std::cout << "  Vertex shader:   " << app.gl_maxv_ubos << std::endl;
    std::cout << "  Geometry shader: " << app.gl_maxg_ubos << std::endl;
    std::cout << "  Fragment shader: " << app.gl_maxf_ubos << std::endl;
    std::cout << "  Compute shader:  " << app.gl_maxc_ubos << '\n' << std::endl;

    std::cout << "$ Maximum allowed number of shader storage buffers: " << '\n' << std::endl;
    std::cout << "  Fragment shader: " << app.gl_maxf_ssbos << std::endl;
    std::cout << "  Compute shader:  " << app.gl_maxc_ssbos << '\n' << std::endl;

    std::cout << "$ GPGPU limitation of compute shaders: " << '\n' << std::endl;
    std::cout << "  Max number of invocations (threads): " << app.cs_max_invocations << std::endl;
    std::cout << "  Max work group count (x, y, z): " << app.cs_nx << ", " << app.cs_ny << ", " << app.cs_nz << std::endl;
    std::cout << "  Max work group size  (x, y, z): " << app.cs_sx << ", " << app.cs_sy << ", " << app.cs_sz << std::endl;
    std::cout << '\n' << std::endl;
	return true;
}
bool openglApp::InitOpengl()
{
    return true;
}
void openglApp::CalculateFrameStats()
{
    // 该代码计算每秒帧速，并计算每一帧渲染需要的时间，显示在窗口标题
    static int frameCnt = 0;
    static float timeElapsed = 0.0f;
    frameCnt++;
    if ((glfwGetTime() - timeElapsed) >= 1.0f)
    {
        float fps = (float)frameCnt; // fps = frameCnt / 1
        float mspf = 1000.0f / fps;
        std::stringstream outs;
        outs.precision(6);
        outs  << "FPS: " << fps << "    "
            << "Frame Time: " << mspf << " (ms)";
        glfwSetWindowTitle(window,outs.str().c_str());
        frameCnt = 0;
        timeElapsed += 1.0f;
    }
}
