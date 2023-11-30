#include "MouseCameraController.hpp"

void RAC::MouseCameraController::control(double dx, double dy, float dt, RACGameObject& gameObject)
{
	//if(dx != 0.0 && dy != 0.0)
	//	std::cout << dx << " " << dy << std::endl;
	glm::vec3 rotate{ 0 };
	rotate.x += dy;
	rotate.y -= dx;

	gameObject.transform.rotation += lookSpeed * rotate;
	

	// limit pitch values between about +/- 85ish degrees
	gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
	gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());
}
