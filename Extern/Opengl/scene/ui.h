#pragma once

#include <string>
#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <imgui/IconsForkAwesome.h>
#include "../scene/entity.h"

namespace scene::ui {

    extern ImFont* truetype_font;  // TrueType, Lato-Regular, 18pt (main font)
    extern ImFont* opentype_font;  // OpenType, Palatino Linotype, 17pt (sub font)
    extern ImFont* web_icon_font;  // Fork Awesome web icon font, 18pt

    enum class Gizmo : uint8_t {
        None, Translate, Rotate, Scale, Bounds
    };

    void Init(void);
    void Clear(void);

    void NewFrame(void);
    void EndFrame(void);

    bool NewInspector(void);
    void EndInspector(void);

    // scene-level helper functions
    void DrawVerticalLine(void);
    void DrawTooltip(const char* desc, float spacing = 5.0f);
    void DrawGizmo(Entity& camera, Entity& target, Gizmo z, std::pair<int, int>pos, std::pair<int, int>size);
    void PushRotation();
    void PopRotation(float radians, bool ccw = false);

    int DrawPopupModal(const char* title, const char* message, const ImVec2& size);
    glm::ivec2 GetCursorPosition();


}
