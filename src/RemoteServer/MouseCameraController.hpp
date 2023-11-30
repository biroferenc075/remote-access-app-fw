
#include "../RAC-Core/RAC_gameobject.hpp"
#include "../RAC-Core/RAC_window.hpp"
namespace RAC {
    class MouseCameraController {
    public:
        MouseCameraController() {}
        void control(double dx, double dy, float dt, RACGameObject& gameObject);
        float lookSpeed{2.0f };
    };
}