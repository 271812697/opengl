#include "../pch.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "event.h"
#include "input.h"
#include "log.h"
#include "window.h"
#include "../scene/renderer.h"
namespace core {
    #pragma warning(push)
    #pragma warning(disable : 4100)
    #pragma warning(disable : 4244)
    #pragma region { event callbacks (GLFW) }

    void Event::GlfwError(int error, const char* description) {
        CORE_ERROR("GLFW error detected (code {0}): {1}", error, description);
    }

    void Event::GlfwCursorEnter(GLFWwindow* window, int entered) {
        if (entered) {
            CORE_INFO("Cursor enters window");
        }
        else {
            CORE_INFO("Cursor leaves window");
        }
    }

    void Event::GlfwCursorPos(GLFWwindow* window, double xpos, double ypos) {
        // x and y are measured in screen coordinates relative to the top-left corner of the window
        if (Window::layer == Layer::Scene) {
            Input::SetCursor(xpos, ypos);
        }
        else if (Window::layer == Layer::ImGui) {
            // when cursor is locked to the window, GLFW will take care of cursor position
            // and offset calculation behind the scenes. Unlike with freeglut, in this case
            // ImGui will handle cursor updates in `NewFrame()` by reading the GLFW backend
            // data, so there's nothing we need to do here...
        }
    }

    void Event::GlfwMouseButton(GLFWwindow* window, int button, int action, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            Input::SetMouseDown(MouseButton::Left, action == GLFW_PRESS);
        }
        else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            Input::SetMouseDown(MouseButton::Right, action == GLFW_PRESS);
        }
        else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
            Input::SetMouseDown(MouseButton::Middle, action == GLFW_PRESS);
        }
    }

    void Event::GlfwScroll(GLFWwindow* window, double xoffset, double yoffset) {
        if (Window::layer == Layer::ImGui) {
           // ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
        }
        else if (Window::layer == Layer::Scene) {
            // unlike a touchpad, a mouse wheel only receives vertical offset
            Input::SetScroll(yoffset);
        }
    }

    void Event::GlfwKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
        // when the Win32 layer is on top, yield input control to the operating system
        if (Window::layer == Layer::Win32) {
            return;
        }

        // remap the key code in GLFW to our standard key code (Win32 and ASCII)
        unsigned char _key = '0';
        switch (key) {
            case GLFW_KEY_UP:     case GLFW_KEY_W:         _key = 'w';        break;
            case GLFW_KEY_DOWN:   case GLFW_KEY_S:         _key = 's';        break;
            case GLFW_KEY_LEFT:   case GLFW_KEY_A:         _key = 'a';        break;
            case GLFW_KEY_RIGHT:  case GLFW_KEY_D:         _key = 'd';        break;
            case GLFW_KEY_Z:                               _key = 'z';        break;
            case GLFW_KEY_R:                               _key = 'r';        break;
            case GLFW_KEY_SPACE:                           _key = VK_SPACE;   break;
            case GLFW_KEY_ENTER:  case GLFW_KEY_KP_ENTER:  _key = VK_RETURN;  break;
            case GLFW_KEY_ESCAPE:                          _key = VK_ESCAPE;  break;
            default:
                return;  // keys other than these registered above are ignored
        }

        if (_key == VK_ESCAPE || _key == VK_RETURN) {
            // functional keys releasing events are handled by the application
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                Input::SetKeyDown(_key, true);
                exit(0);
            }
            return;
        }

        if (Window::layer == Layer::Scene) {
            Input::SetKeyDown(_key, action != GLFW_RELEASE);
        }
        else if (Window::layer == Layer::ImGui) {
            //ImGui_ImplGlfw_KeyCallback(Window::window_ptr, key, scancode, action, mods);
        }
    }

    void Event::GlfwWindowSize(GLFWwindow* window, int width, int height) {
        // this callback is triggered when a window gets resized, however, it receives the new
        // sizes in screen coordinates, not pixels, so we should not resize the viewport here.
    }

    void Event::GlfwFramebufferSize(GLFWwindow* window, int width, int height) {
        // this callback is triggered when the framebuffer of a window gets resized.
        // the new sizes are in pixels, relative to the top-left corner of the content area
        Window::Resize();

        // ImGui will handle display size in `NewFrame()` automatically for us by
        // reading the GLFW backend data, so there's nothing we need to do here...
    }

    void Event::GlfwWindowFocus(GLFWwindow* window, int focused) {
        if (focused) {
            CORE_INFO("Window gains input focus");
        }
        else {
            CORE_INFO("Window loses input focus");
        }
    }

    #pragma endregion  ...

    #pragma warning(pop)

    void Event::RegisterCallbacks() {
       {
            auto* w_ptr = Window::window_ptr;
            CORE_ASERT(w_ptr, "Unable to register callbacks, a window must be created first!");

            glfwSetErrorCallback           (GlfwError);
            glfwSetCursorEnterCallback     (w_ptr, GlfwCursorEnter);
            glfwSetCursorPosCallback       (w_ptr, GlfwCursorPos);
            glfwSetMouseButtonCallback     (w_ptr, GlfwMouseButton);
            glfwSetScrollCallback          (w_ptr, GlfwScroll);
            glfwSetKeyCallback             (w_ptr, GlfwKey);
            glfwSetWindowSizeCallback      (w_ptr, GlfwWindowSize);
            glfwSetFramebufferSizeCallback (w_ptr, GlfwFramebufferSize);
            glfwSetWindowFocusCallback     (w_ptr, GlfwWindowFocus);
        }
    }

}
