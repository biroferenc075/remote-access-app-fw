
#include "../BFE-Core/BFE_gameobject.hpp"
#include "../BFE-Core/BFE_window.hpp"
namespace BFE {
    class MouseCameraController {
    public:
        MouseCameraController() {}
        void control(double dx, double dy, float dt, BFEGameObject& gameObject);
        float lookSpeed{2.0f };
    };
}