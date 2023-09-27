#include "MyApplication.hpp"
#include "../BFE-Core/BFE_render_system.hpp"

void Example::MyApplication::onStart() {
    std::cout << "start" << std::endl;
    globalPool = BFEDescriptorPool::Builder(bfeDevice)
        .setMaxSets(MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT)
        .build();
    loadGameObjects();
}
void Example::MyApplication::onConn() {
    connected = true;
    std::cout << "conn" << std::endl;
    uboBuffers = std::vector<std::unique_ptr<BFEBuffer>>(MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < uboBuffers.size(); i++) {
        uboBuffers[i] = std::make_unique<BFEBuffer>(
            bfeDevice, sizeof(UBO), MAX_FRAMES_IN_FLIGHT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        uboBuffers[i]->map();
    }

    globalDescriptorSets = std::vector<VkDescriptorSet>(MAX_FRAMES_IN_FLIGHT);
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
    
    //camera.setViewDirection(glm::bfec3(0.f), glm::bfec3(0.5, 0.f, 1.f));
    camera.setViewTarget(glm::vec3(0.f, 5.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

    viewerObject = BFEGameObject::createGameObject();
    cameraController = KeyboardCameraController();
}
void Example::MyApplication::onDisconn() {

}

void Example::MyApplication::onUpdate(float totalTime, float deltaTime) {
    std::cout << "upd" << std::endl;
    gameObjects[0].transform.translation = glm::vec3(0.8 * glm::sin(totalTime), 0.6f, 0.8 * glm::cos(totalTime) + 2.5f);
    gameObjects[0].transform.rotation.y += deltaTime * 2.5f;

}
void Example::MyApplication::onDraw(float totalTime, float deltaTime) {
    std::cout << "draw" << std::endl;
    std::vector<int> chars{};
    //cameraController.moveInPlaneXZ(chars, frameTime, viewerObject, t);
    camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

    float aspect = bfeRenderer.getAspectRatio();
    camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);
    if (auto commandBuffer = bfeRenderer.beginFrame()) {
        int frameIndex = bfeRenderer.getFrameIndex();
        FrameInfo frameInfo{ frameIndex, deltaTime, commandBuffer, camera, globalDescriptorSets[frameIndex] };
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
}
void Example::MyApplication::onKey() {

}
void Example::MyApplication::onMouse() {

}
void Example::MyApplication::onWindowEvent() {

}


void Example::MyApplication::loadGameObjects() {
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
