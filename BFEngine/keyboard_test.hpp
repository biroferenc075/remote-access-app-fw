#pragma once

#include "BFE_gameobject.hpp"
#include "BFE_window.hpp"

namespace BFE {
    class KeyboardController {
    public:
        struct KeyMappings {
            int mobfeLeft = GLFW_KEY_A;
            int mobfeRight = GLFW_KEY_D;
            int mobfeForward = GLFW_KEY_W;
            int mobfeBackward = GLFW_KEY_S;
            int mobfeUp = GLFW_KEY_E;
            int mobfeDown = GLFW_KEY_Q;
            int lookLeft = GLFW_KEY_LEFT;
            int lookRight = GLFW_KEY_RIGHT;
            int lookUp = GLFW_KEY_UP;
            int lookDown = GLFW_KEY_DOWN;
        };
        void mobfeInPlaneXZ(GLFWwindow* window, float dt, BFEGameObject& gameObject);
        KeyMappings keys{};
        float mobfeSpeed{ 3.f };
        float lookSpeed{ 1.5f };
    };
}