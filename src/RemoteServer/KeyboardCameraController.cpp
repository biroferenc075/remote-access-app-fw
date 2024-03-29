#include "KeyboardCameraController.hpp"


namespace RAC {
	void KeyboardCameraController::moveInPlaneXZ(GLFWwindow* window, float dt, RACGameObject& gameObject) {

		glm::vec3 rotate{ 0 };
		if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) rotate.y += 1.f;
		if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) rotate.y -= 1.f;
		if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) rotate.x += 1.f;
		if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) rotate.x -= 1.f;

		if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
			gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
		}

		// limit pitch values between about +/- 85ish degrees
		gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
		gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

		float yaw = gameObject.transform.rotation.y;
		const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };
		const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
		const glm::vec3 upDir{ 0.f, -1.f, 0.f };

		glm::vec3 moveDir{ 0.f };
		if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
		if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
		if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
		if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
		if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
		if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;

		if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
			gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
		}
	}
	void KeyboardCameraController::moveInPlaneXZ(std::set<int> keysPressed, float dt, RACGameObject& gameObject, float t = 0) {
		glm::vec3 rotate{0};

		if (keysPressed.find(keys.lookRight) != keysPressed.end()) rotate.y += 1.f;
		if (keysPressed.find(keys.lookLeft) != keysPressed.end()) rotate.y -= 1.f;
		if (keysPressed.find(keys.lookUp) != keysPressed.end()) rotate.x += 1.f;
		if (keysPressed.find(keys.lookDown) != keysPressed.end()) rotate.x -= 1.f;

		if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
			gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
		}

		// limit pitch values between about +/- 85ish degrees
		gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
		gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

		float yaw = gameObject.transform.rotation.y;
		const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };
		const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
		const glm::vec3 upDir{ 0.f, -1.f, 0.f };

		glm::vec3 moveDir{ 0.f };
		if (keysPressed.find(keys.moveForward) != keysPressed.end()) moveDir += forwardDir;
		if (keysPressed.find(keys.moveBackward) != keysPressed.end()) moveDir -= forwardDir;
		if (keysPressed.find(keys.moveRight) != keysPressed.end()) moveDir += rightDir;
		if (keysPressed.find(keys.moveLeft) != keysPressed.end()) moveDir -= rightDir;
		if (keysPressed.find(keys.moveUp) != keysPressed.end())  moveDir += upDir;
		if (keysPressed.find(keys.moveDown) != keysPressed.end()) moveDir -= upDir;
		
		if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
			gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
		}
		//gameObject.transform.translation = glm::vec3(5.0 * glm::sin(t), -0.6f, 5.0 * glm::cos(t)+0.2f);
		//gameObject.transform.rotation = glm::vec3(0, t, 0);
		
	}

}