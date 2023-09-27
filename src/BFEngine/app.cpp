#include "app.hpp"
#include "../BFE-Core/BFE_camera.hpp"
#include "../BFE-Core/BFE_render_system.hpp"
#include "../BFE-Core/BFE_buffer.hpp"
#include "keyboard_test.hpp"
#include "consts.hpp"

#include <stdexcept>
#include <array>
#include <numeric>

#include <Windows.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>


namespace BFE {

    struct UBO {
        glm::mat4 projectionView{ 1.f };
        glm::vec3 lightDir = glm::normalize(glm::vec3{ 1.f, -3.f, -1.f });
    };
	App::App(boost::lockfree::queue<BFE::Frame*>& frameQueue) : frameQueue(frameQueue) {
        globalPool = BFEDescriptorPool::Builder(bfeDevice)
            .setMaxSets(MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT)
            .build();
		loadGameObjects();
	}

	App::~App() {}

    auto currentTime = std::chrono::high_resolution_clock::now();
    void App::run() {
        std::vector<std::unique_ptr<BFEBuffer>> uboBuffers(MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); i++ ){
            uboBuffers[i] = std::make_unique<BFEBuffer>(
                bfeDevice, sizeof(UBO), MAX_FRAMES_IN_FLIGHT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffers[i]->map();
        }

        auto globalSetLayout = BFEDescriptorSetLayout::Builder(bfeDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

        std::vector<VkDescriptorSet> globalDescriptorSets(MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = gameObjects[0].texture->textureImageView;
            imageInfo.sampler = gameObjects[0].texture->textureSampler;

            BFEDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .writeImage(1, &imageInfo)
                .build(globalDescriptorSets[i]);
        }

        BFEBuffer UBOBuffer{
            bfeDevice, sizeof(UBO), MAX_FRAMES_IN_FLIGHT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, bfeDevice.properties.limits.minUniformBufferOffsetAlignment, };
        UBOBuffer.map();

        BFERenderSystem renderSystem(bfeDevice, bfeRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());
        BFECamera camera{};
        //camera.setViewDirection(glm::bfec3(0.f), glm::bfec3(0.5, 0.f, 1.f));
        camera.setViewTarget(glm::vec3(0.f, 5.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

        auto viewerObject = BFEGameObject::createGameObject();
        KeyboardController cameraController{};

        float t = 0;

        while (!bfeWindow.shouldClose()) {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            t += frameTime;
            currentTime = newTime;

            gameObjects[0].transform.translation = glm::vec3(0.8 * glm::sin(t), 0.6f, 0.8 * glm::cos(t)+2.5f);
            gameObjects[0].transform.rotation.y += frameTime * 2.5f;

            std::vector<int> chars{};
            //cameraController.moveInPlaneXZ(chars, frameTime, viewerObject, t);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);
            
            float aspect = bfeRenderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);
            if (auto commandBuffer = bfeRenderer.beginFrame()) {
                int frameIndex = bfeRenderer.getFrameIndex();
                FrameInfo frameInfo{ frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex]};
                // update
                UBO ubo{};
                ubo.projectionView = camera.getProjection() * camera.getView();
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();


                //rendering
                bfeRenderer.beginSwapChainRenderPass(commandBuffer);
                renderSystem.renderGameObjects(frameInfo, gameObjects);
                bfeRenderer.endSwapChainRenderPass(commandBuffer);
                bfeRenderer.endFrame();
            }
            Sleep(1000.0f / FRAMERATE - frameTime);
        }

        vkDeviceWaitIdle(bfeDevice.device());
    }
	void App::loadGameObjects() {
        std::shared_ptr<BFEModel> bfeModel = BFEModel::createModelFromFile(pid, bfeDevice, "assets/models/Scaniverse.obj");
        std::shared_ptr<BFETexture> bfeTexture = BFETexture::createTextureFromFile(pid, bfeDevice, "assets/textures/Scaniverse.jpg");
        auto obj = BFEGameObject::createGameObject();
        obj.model = bfeModel;
        obj.texture = bfeTexture;
        obj.transform.translation = { 0.0f, 0.0f, 2.5f };
        obj.transform.scale = { 1.5f, 1.5f, 1.5f };
        obj.transform.rotation = { 3.14f, 0.0f, 0.0f };

        gameObjects.push_back(std::move(obj));
    }
}