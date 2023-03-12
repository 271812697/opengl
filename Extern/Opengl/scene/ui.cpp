#include "../pch.h"

#define IMGUI_DISABLE_METRICS_WINDOW
#define IMGUI_DEFINE_MATH_OPERATORS

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include<imgui/IconsForkAwesome.h>
#include<imgui/backends/imgui_impl_glfw.h>
#include<imgui/backends/imgui_impl_opengl3.h>
#include<imgui/ImGuizmo.h>
#include <imgui/imstb_rectpack.h>
#include <imgui/imstb_textedit.h>
#include <imgui/imstb_truetype.h>

#include "../core/base.h"
#include "../core/clock.h"
#include "../core/input.h"
#include "../core/log.h"
#include "../core/window.h"
#include "../component/all.h"
#include "../scene/entity.h"
#include "../scene/renderer.h"
#include "../scene/ui.h"
#include "../util/path.h"
#include "../scene/scene.h"

using namespace core;
using namespace component;
using namespace ImGui;
#define VK_SPACE          0x20
#define VK_RETURN         0x0D
#define VK_ESCAPE         0x1B
namespace scene::ui {

    ImFont* truetype_font;  // TrueType, Lato-Regular, 18pt (main font)
    ImFont* opentype_font;  // OpenType, Palatino Linotype, 17pt (sub font)
    ImFont* web_icon_font;  // Fork Awesome web icon font, 18pt

    // private global variables
    static ImVec2 window_center;
    static ImVec4 red    = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    static ImVec4 yellow = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    static ImVec4 green  = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    static ImVec4 blue   = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
    static ImVec4 cyan   = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);

    static const ImGuiColorEditFlags color3_flags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha;
    static const ImGuiWindowFlags invisible_window_flags = ImGuiWindowFlags_NoBackground
        | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs;
    static int rotation_start_index = 0;

    void Init() {
        CreateContext();
        ImGuiIO& io = GetIO();
        ImGuiStyle& style = GetStyle();


        window_center = ImVec2((float)Window::width, (float)Window::height) * 0.5f;

        // load fonts from the resource folder
        float fontsize_main = 18.0f;
        float fontsize_icon = 18.0f;  // bake icon font into the main font
        float fontsize_sub  = 17.0f;

        std::string path = "res//font//";
        std::string ttf_main = path+"Lato.ttf";
        std::string ttf_sub  = path+"palatino.ttf";
        std::string ttf_icon = path + FONT_ICON_FILE_NAME_FK;

        ImFontConfig config_main;
        config_main.PixelSnapH = true;
        config_main.OversampleH = 4;
        config_main.OversampleV = 4;
        config_main.RasterizerMultiply = 1.2f;  // brighten up the font to make them more readable
        config_main.GlyphExtraSpacing.x = 0.0f;

        ImFontConfig config_sub;
        config_sub.PixelSnapH = true;
        config_sub.OversampleH = 4;
        config_sub.OversampleV = 4;
        config_sub.RasterizerMultiply = 1.25f;  // brighten up the font to make them more readable
        config_sub.GlyphExtraSpacing.x = 0.0f;

        ImFontConfig config_icon;
        config_icon.MergeMode = true;
        config_icon.PixelSnapH = true;
        config_icon.OversampleH = 4;
        config_icon.OversampleV = 4;
        config_icon.RasterizerMultiply = 1.5f;  // brighten up the font to make them more readable
        config_icon.GlyphOffset.y = 0.0f;       // tweak this to vertically align with the main font
        config_icon.GlyphMinAdvanceX = fontsize_main;  // enforce monospaced icon font
        config_icon.GlyphMaxAdvanceX = fontsize_main;  // enforce monospaced icon font

        static const ImWchar icon_ranges[] = { ICON_MIN_FK, ICON_MAX_FK, 0 };  // zero-terminated
        
        truetype_font = io.Fonts->AddFontFromFileTTF(ttf_main.c_str(), fontsize_main, &config_main);
        web_icon_font = io.Fonts->AddFontFromFileTTF(ttf_icon.c_str(), fontsize_icon, &config_icon, icon_ranges);
        opentype_font = io.Fonts->AddFontFromFileTTF(ttf_sub.c_str(), fontsize_sub, &config_sub);

        // build font textures
        unsigned char* pixels;
        int width, height, bytes_per_pixel;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytes_per_pixel);

        // load default dark theme
        StyleColorsDark();

        // setup custom styles
        style.WindowBorderSize         = 0.0f;
        style.FrameBorderSize          = 1.0f;
        style.PopupBorderSize          = 1.0f;
        style.ChildBorderSize          = 1.0f;
        style.TabBorderSize            = 0.0f;
        style.ScrollbarSize            = 18.0f;
        style.GrabMinSize              = 10.0f;

        style.WindowPadding            = ImVec2(8.0f, 8.0f);
        style.FramePadding             = ImVec2(4.0f, 6.0f);
        style.ItemSpacing              = ImVec2(10.0f, 10.0f);
        style.ItemInnerSpacing         = ImVec2(10.0f, 10.0f);
        style.IndentSpacing            = 16.0f;

        style.WindowRounding           = 0.0f;
        style.ChildRounding            = 0.0f;
        style.FrameRounding            = 4.0f;
        style.PopupRounding            = 0.0f;
        style.TabRounding              = 4.0f;
        style.GrabRounding             = 4.0f;
        style.ScrollbarRounding        = 12.0f;

        style.WindowMenuButtonPosition = ImGuiDir_Left;
        style.ColorButtonPosition      = ImGuiDir_Right;

        style.ButtonTextAlign          = ImVec2(0.5f, 0.5f);
        style.WindowTitleAlign         = ImVec2(0.0f, 0.5f);
        style.SelectableTextAlign      = ImVec2(0.0f, 0.0f);

        style.AntiAliasedLines         = true;
        style.AntiAliasedFill          = true;
        style.AntiAliasedLinesUseTex   = true;

        // setup custom colors
        auto& c = GetStyle().Colors;

        c[ImGuiCol_WindowBg]             = ImVec4(0.0f, 0.0f, 0.0f, 0.85f);
        c[ImGuiCol_ChildBg]              = ImVec4(0.1f, 0.1f, 0.1f, 0.85f);
        c[ImGuiCol_PopupBg]              = ImVec4(0.1f, 0.1f, 0.1f, 0.85f);

        c[ImGuiCol_FrameBg]              = ImVec4(0.0f, 0.0f, 0.0f, 0.75f);
        c[ImGuiCol_FrameBgHovered]       = ImVec4(0.2f, 0.2f, 0.2f, 0.75f);
        c[ImGuiCol_FrameBgActive]        = ImVec4(0.3f, 0.3f, 0.3f, 0.75f);

        c[ImGuiCol_TitleBg]              = ImVec4(0.2f, 0.2f, 0.2f, 0.75f);
        c[ImGuiCol_TitleBgActive]        = ImVec4(0.0f, 0.3f, 0.0f, 0.9f);
        c[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.0f, 0.0f, 0.0f, 0.75f);

        c[ImGuiCol_ScrollbarBg]          = ImVec4(0.0f, 0.0f, 0.0f, 0.75f);
        c[ImGuiCol_ScrollbarGrab]        = ImVec4(0.2f, 0.2f, 0.2f, 0.9f);
        c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.3f, 0.3f, 0.3f, 0.9f);
        c[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.4f, 0.4f, 0.4f, 0.9f);

        c[ImGuiCol_CheckMark]            = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
        c[ImGuiCol_SliderGrab]           = ImVec4(0.0f, 0.4f, 0.0f, 0.9f);
        c[ImGuiCol_SliderGrabActive]     = ImVec4(0.0f, 0.5f, 0.0f, 0.9f);

        c[ImGuiCol_Button]               = ImVec4(0.0f, 0.3f, 0.0f, 0.9f);
        c[ImGuiCol_ButtonHovered]        = ImVec4(0.0f, 0.55f, 0.0f, 0.9f);
        c[ImGuiCol_ButtonActive]         = ImVec4(0.0f, 0.5f, 0.0f, 0.9f);

        c[ImGuiCol_Header]               = ImVec4(0.5f, 0.0f, 1.0f, 0.5f);
        c[ImGuiCol_HeaderHovered]        = ImVec4(0.5f, 0.0f, 1.0f, 0.8f);
        c[ImGuiCol_HeaderActive]         = ImVec4(0.5f, 0.0f, 1.0f, 0.7f);

        c[ImGuiCol_Tab]                  = ImVec4(0.0f, 0.3f, 0.0f, 0.8f);
        c[ImGuiCol_TabHovered]           = ImVec4(0.0f, 0.4f, 0.0f, 0.8f);
        c[ImGuiCol_TabActive]            = ImVec4(0.0f, 0.4f, 0.0f, 0.8f);
        c[ImGuiCol_TabUnfocused]         = ImVec4(0.2f, 0.2f, 0.2f, 0.9f);
        c[ImGuiCol_TabUnfocusedActive]   = ImVec4(0.2f, 0.2f, 0.2f, 0.9f);

        ImGui_ImplGlfw_InitForOpenGL(Window::window_ptr, true);
        ImGui_ImplOpenGL3_Init();
    }

    void Clear() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        DestroyContext();
    }

    void NewFrame() {
        if constexpr (_freeglut) {

        }
        else {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();  // for GLFW backend we need to call this manually
            ImGuizmo::BeginFrame();
        }
    }

    void EndFrame() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(GetDrawData());
    }

    bool NewInspector(void) {
        static const float w = 256.0f * 1.25f;  // tweaked for 1600 x 900 resolution
        static const float h = 612.0f * 1.25f;

        SetNextWindowPos(ImVec2(Window::width - w, (Window::height - h) * 0.5f));
        SetNextWindowSize(ImVec2(w, h));

        static ImGuiWindowFlags inspector_flags = ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

        PushID("Inspector Window");

        if (Begin(ICON_FK_LOCATION_ARROW " Inspector", 0, inspector_flags)) {
            return true;
        }

        CORE_ERROR("Failed to load inspector due to clipping issues...");
        CORE_ERROR("Did you draw a full screen opaque window?");
        return false;
    }

    void EndInspector(void) {
        End();
        PopID();
    }

    void DrawVerticalLine() {
        SeparatorEx(ImGuiSeparatorFlags_Vertical);
    }

    void DrawTooltip(const char* desc, float spacing) {
        SameLine(0.0f, spacing);
        TextDisabled("(?)");

        if (IsItemHovered()) {
            PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            PushStyleColor(ImGuiCol_PopupBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            BeginTooltip();
            PushTextWrapPos(GetFontSize() * 35.0f);
            TextUnformatted(desc);
            PopTextWrapPos();
            EndTooltip();
            PopStyleColor(2);
        }
    }



    void DrawGizmo(Entity& camera, Entity& target, Gizmo z) {
        static const ImVec2 win_pos = ImVec2(0.0f, 50.0f);
        static const ImVec2 win_size = ImVec2((float)Window::width, (float)Window::height - 82.0f);

        ImGuizmo::MODE mode = ImGuizmo::MODE::LOCAL;
        ImGuizmo::OPERATION operation = ImGuizmo::OPERATION::TRANSLATE;

        switch (z) {
            case Gizmo::Translate: {
                operation = ImGuizmo::OPERATION::TRANSLATE;
                break;
            }
            case Gizmo::Rotate: {
                operation = ImGuizmo::OPERATION::ROTATE;
                break;
            }
            case Gizmo::Scale: {
                operation = ImGuizmo::OPERATION::SCALE;
                break;
            }
            case Gizmo::Bounds: case Gizmo::None: default: {
                return;
            }
        }

        auto& T = target.GetComponent<Transform>();
        auto& C = camera.GetComponent<Camera>();
        glm::mat4 V = C.GetViewMatrix();
        glm::mat4 P = C.GetProjectionMatrix();

        // convert model matrix to left-handed as ImGuizmo assumes a left-handed coordinate system
        static const glm::vec3 RvL = glm::vec3(1.0f, 1.0f, -1.0f);  // scaling vec for R2L and L2R
        glm::mat4 transform = glm::scale(T.transform, RvL);

        SetNextWindowPos(win_pos);    // below the menu bar
        SetNextWindowSize(win_size);  // above the status bar
        Begin("##Invisible Gizmo Window", 0, invisible_window_flags);

        ImGuizmo::SetOrthographic(true);
        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(win_pos.x, win_pos.y, win_size.x, win_size.y);
        ImGuizmo::Manipulate(value_ptr(V), value_ptr(P), operation, mode, value_ptr(transform));

        // if the gizmo is being manipulated, which means that the transform matrix may have been
        // (but not necessarily) modified by the gizmo from UI, we update the transform component.
        // note that guizmos are supposed to be drawn and manipulated one at a time, if multiple
        // gizmos for multiple objects are rendered at the same time, trying to manipulate one of
        // them will affect all the others because `ImGuizmo::Manipulate` will not work properly.
        // in addition `ImGuizmo::IsUsing` can't tell which specific object is being used, it has
        // only one global context that applies to all objects whose guizmo is currently rendered

        if (ImGuizmo::IsUsing()) {
            transform = glm::scale(transform, RvL);  // convert back to right-handed
            T.SetTransform(transform);
        }

        ImGui::End();
    }

    void PushRotation() {
        rotation_start_index = GetWindowDrawList()->VtxBuffer.Size;
    }

    void PopRotation(float radians, bool ccw) {
        // vertex buffer lower and upper bounds (p_min, p_max)
        static const float max_float = std::numeric_limits<float>::max();
        ImVec2 lower(+max_float, +max_float);
        ImVec2 upper(-max_float, -max_float);

        auto& buff = GetWindowDrawList()->VtxBuffer;
        for (int i = rotation_start_index; i < buff.Size; ++i) {
            lower = ImMin(lower, buff[i].pos);
            upper = ImMax(upper, buff[i].pos);
        }

        // use the buffer center as rotation's pivot point
        ImVec2 center = ImVec2((lower.x + upper.x) / 2, (lower.y + upper.y) / 2);

        float s = sin(radians);
        float c = cos(radians);
        
        for (int i = rotation_start_index; i < buff.Size; ++i) {
            ImVec2 offset = buff[i].pos - center;  // vertex offset relative to the center pivot
            ImVec2 offset_new = ccw ? ImRotate(offset, s, c) : ImRotate(offset, c, s);  // rotate about center
            buff[i].pos = center + offset_new;  // update rotated vertex position (center stays put)
        }
    }

    int DrawPopupModal(const char* title, const char* message, const ImVec2& size) {
        int button_id = -1;  // none of the buttons was pressed
        OpenPopup(title);

        ImVec2 center = GetMainViewport()->GetCenter();
        SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        SetNextWindowSize(size);

        PushStyleVar(ImGuiStyleVar_WindowTitleAlign, ImVec2(0.5f, 0.5f));
        PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 8.0f));

        float button_indentation = size.x * 0.1f;
        float text_width = CalcTextSize(message).x;
        float text_indentation = (size.x - text_width) * 0.5f;  // center the text locally to the window

        // if text too long to be drawn on one line, enforce a minimum indentation
        text_indentation = std::max(text_indentation, 20.0f);

        if (BeginPopupModal(title, NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            SameLine(text_indentation);
            PushTextWrapPos(size.x - text_indentation);
            TextWrapped(message);
            PopTextWrapPos();
            Separator();

            static const ImVec2 vspacing = ImVec2(0.0f, 0.1f);
            Dummy(vspacing);

            if (Indent(button_indentation); true) {
                if (Button("OK", ImVec2(100.0f, 0.0f))) {
                    CloseCurrentPopup();
                    button_id = 0;  // OK
                }

                SetItemDefaultFocus();
                SameLine(0.0f, size.x - 2.0f * (100.0f + button_indentation));

                if (Button("Cancel", ImVec2(100.0f, 0.0f))) {
                    CloseCurrentPopup();
                    button_id = 1;  // cancel
                }
            }

            Unindent(button_indentation);
            Dummy(vspacing);
            EndPopup();
        }

        PopStyleVar(2);
        return button_id;
    }

    glm::ivec2 GetCursorPosition() {
        if (Window::layer == Layer::Scene) {
            return Input::GetCursorPosition();
        }

        auto mouse_pos = ImGui::GetMousePos();
        return glm::ivec2(mouse_pos.x, mouse_pos.y);
    }
}
