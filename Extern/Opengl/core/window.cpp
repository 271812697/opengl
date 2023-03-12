#include <windows.h>
#include <gdiplus.h>
#include <gdiplusinit.h>
#include <atlimage.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "clock.h"
#include "input.h"
#include "log.h"
#include "window.h"
#include "../util/path.h"
namespace core {

    static HWND hWND = nullptr;  // Win32 internal window handle

    void Window::Init() {
        width = 1600;
        height = 900;
        {
            glfwInit();
            const GLFWvidmode* vmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            pos_x = (vmode->width - width) / 2;
            pos_y = (vmode->height - height) / 2;

            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_SAMPLES, 4);  // enforce 4 samples per pixel MSAA

            if constexpr (debug_mode) {
                glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);  // hint the debug context
            }

            window_ptr = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
            CORE_ASERT(window_ptr != nullptr, "Failed to create the main window...");
            CORE_INFO("Window resolution is set to {0}x{1} ...", width, height);

            glfwSetWindowPos(window_ptr, pos_x, pos_y);
            glfwSetWindowAspectRatio(window_ptr, 16, 9);
            glfwSetWindowAttrib(window_ptr, GLFW_RESIZABLE, GLFW_FALSE);

            glfwMakeContextCurrent(window_ptr);
            glfwSwapInterval(0);  // disable vsync because we want to test performance

            // glad: load all OpenGL function pointers
// ---------------------------------------
            if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
            {
                CORE_ERROR("fail to load glfw function");
               
            }

        }

        // retrieve the global handle of our window, will be used by screenshots later
        hWND = ::FindWindow(NULL, (LPCWSTR)(L"opengl"));

        // disable max/min/close button on the title bar
        LONG style = GetWindowLong(hWND, GWL_STYLE) ^ WS_SYSMENU;
        SetWindowLong(hWND, GWL_STYLE, style);
    }

    void Window::Clear() {
        {
            if (window_ptr) {
                glfwDestroyWindow(window_ptr);
                glfwTerminate();
                window_ptr = nullptr;
            }
        }
    }

    void Window::Rename(const std::string& new_title) {
        title = new_title;
        {
            glfwSetWindowTitle(window_ptr, title.c_str());
        }
    }

    void Window::Resize() {
      {
            glfwSetWindowPos(window_ptr, pos_x, pos_y);
            glfwSetWindowSize(window_ptr, width, height);
            glfwSetWindowAspectRatio(window_ptr, 16, 9);
        }

        // viewport position is in pixels, relative to the the bottom-left corner of the window
        glViewport(0, 0, width, height);
    }





}
