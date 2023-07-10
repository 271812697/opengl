#include"../pch.h"
#include "camera.h"
#include "transform.h"
#include "../util/math.h"
#include "Window/InputManager.h"
using namespace utils::math;
namespace component {

    Camera::Camera(Transform* T, View view) :
        Component(),
        fov(45.0f),
        near_clip(0.1f),
        far_clip(100.0f),
        move_speed(5.0f),
        zoom_speed(0.04f),
        rotate_speed(0.3f),
        orbit_speed(0.05f),
        initial_position(T->position),
        initial_rotation(T->rotation),
        T(T), view(view) {}

    glm::mat4 Camera::GetViewMatrix() const {

        if constexpr (true) {
            return glm::inverse(T->transform);
        }
        else {
            return glm::lookAt(T->position, T->position + T->forward, T->up);
        }
    }

    glm::mat4 Camera::GetProjectionMatrix() const {
        return (view == View::Orthgraphic)
            ? glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_clip, far_clip)
            : glm::perspective(glm::radians(fov),aspect, near_clip, far_clip);
    }

    void Camera::Update() {
        auto instance= Windowing::Inputs::InputManager::GetInputManager();
        float deltatime =instance->GetDeltaTime();
        static bool recovering = false;

        
        // smoothly recover camera to its initial position and orientation
        if (recovering) {
            float t = EaseFactor(10.0f, deltatime);
            T->SetPosition(Lerp(T->position, initial_position, t));
            T->SetRotation(SlerpRaw(T->rotation, initial_rotation, t));

            if (Equals(T->position, initial_position) && Equals(T->rotation, initial_rotation)) {
                recovering = false;  // keep recovering until both position and rotation are recovered
            }

            return;  
        }


     

        // key events are only processed if there's no mouse button event (which cannot be interrupted)
        if (instance->IsKeyPressed(Windowing::Inputs::EKey::KEY_R)) {
            
            recovering = true;
        }
        
        if(instance->IsMouseButtonPressed(Windowing::Inputs::EMouseButton::MOUSE_BUTTON_RIGHT)){
        // rotation is limited to the X and Y axis (pitch and yawn only, no roll)
            //instance->GetMousePosition
        auto [x, y] = instance->GetCursoreOffset();
        float euler_y = T->euler_y -x* rotate_speed;
        float euler_x = T->euler_x - y * rotate_speed;

        euler_y = glm::radians(euler_y);
        euler_x = glm::radians(glm::clamp(euler_x, -89.0f, 89.0f));  // clamp vertical rotation
        glm::quat rotation = glm::quat_cast(glm::eulerAngleYXZ(euler_y, euler_x, 0.0f));

        T->SetRotation(rotation);
        }
        
        // translation (not normalized, movement is faster along the diagonal)
        if (instance->IsKeyPressed(Windowing::Inputs::EKey::KEY_W)) {
            T->Translate(T->forward * (move_speed * deltatime));
        }

        if (instance->IsKeyPressed(Windowing::Inputs::EKey::KEY_S)) {
            T->Translate(-T->forward * (move_speed * deltatime));
        }

        if (instance->IsKeyPressed(Windowing::Inputs::EKey::KEY_A)) {
            T->Translate(-T->right * (move_speed * deltatime));
        }

        if (instance->IsKeyPressed(Windowing::Inputs::EKey::KEY_D)) {
            T->Translate(T->right * (move_speed * deltatime));
        }

        if (instance->IsKeyPressed(Windowing::Inputs::EKey::KEY_Z)) {
            T->Translate(-T->up * (move_speed * deltatime));
        }

        if (instance->IsKeyPressed(Windowing::Inputs::EKey::KEY_SPACE)) {  // VK_SPACE
            T->Translate(T->up * (move_speed * deltatime));
        }
    }

}
