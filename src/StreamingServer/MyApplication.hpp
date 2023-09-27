#include "RemoteApplication.hpp"
#include "KeyboardCameraController.hpp"
namespace Example {
	class MyApplication : public RemoteApplication {
	public:
		void onStart();
		void onConn();
		void onDisconn();
		void onUpdate(float totalTime, float deltaTime);
		void onDraw(float totalTime, float deltaTime);
		void onKey();
		void onMouse();
		void onWindowEvent();
	private:
		BFEGameObject viewerObject = BFEGameObject::createGameObject();
		BFECamera camera;
		KeyboardCameraController cameraController;
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
		};
	struct UBO {
		glm::mat4 projectionView{ 1.f };
		glm::vec3 lightDir = glm::normalize(glm::vec3{ 1.f, -3.f, -1.f });
	};
}