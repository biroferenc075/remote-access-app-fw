#include "RemoteApplication.hpp"
#include "KeyboardCameraController.hpp"
#include "MouseCameraController.hpp"
namespace Example {
	class MyApplication : public RemoteApplication {
	public:
		void onStart();
		void onConn();
		void onDisconn();
		void onUpdate(float totalTime, float deltaTime, std::set<int>& keysPressed, std::set<int>& keysReleased, std::set<int>& keysHeld);
		void onDraw(float totalTime, float deltaTime);
		void onKeyPress(int key, int mods, int scancode);
		void onKeyRelease(int key, int mods, int scancode);
		void onMouseMove(double dX, double dY, double x, double y);
		void onMousePress(double x, double y, int button, int mods);
		void onMouseRelease(double x, double y, int button, int mods);
		void onWindowEvent(int x, int y);
	private:
		BFEGameObject viewerObject = BFEGameObject::createGameObject();
		BFECamera camera;
		KeyboardCameraController keyboardCameraController;
		MouseCameraController mouseCameraController;
		std::vector<std::unique_ptr<BFEBuffer>> uboBuffers;
		std::vector<VkDescriptorSet> globalDescriptorSets;
		std::unique_ptr<BFEDescriptorPool> globalPool{};
		std::vector<BFEGameObject> gameObjects;
		std::unique_ptr<BFEDescriptorSetLayout> globalSetLayout = BFEDescriptorSetLayout::Builder(bfeDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		BFERenderSystem renderSystem = BFERenderSystem(bfeDevice, bfeRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());;
		void loadGameObjects();
		glm::vec2 mouseMoveSinceLastFrame{ 0 };
	};
	
	struct UBO {
		glm::mat4 projectionView{ 1.f };
		glm::vec3 lightDir = glm::normalize(glm::vec3{ 1.f, -3.f, -1.f });
	};
}