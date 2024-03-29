#include "MyApplication.hpp"
#include "../RAC-Core/RAC_render_system.hpp"

void Example::MyApplication::onStart() {
    globalPool = RACDescriptorPool::Builder(racDevice)
        .setMaxSets(MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT)
        .build();
    loadGameObjects();
    std::cout << "start" << std::endl;
}
void Example::MyApplication::onConn() {
    connected = true;
    std::cout << "conn" << std::endl;
    if (!started) {
        uboBuffers = std::vector<std::unique_ptr<RACBuffer>>(MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); i++) {
            uboBuffers[i] = std::make_unique<RACBuffer>(
                racDevice, sizeof(UBO), MAX_FRAMES_IN_FLIGHT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffers[i]->map();
        }

        globalDescriptorSets = std::vector<VkDescriptorSet>(MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = gameObjects[0].texture->textureImageView;
            imageInfo.sampler = gameObjects[0].texture->textureSampler;

            RACDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .writeImage(1, &imageInfo)
                .build(globalDescriptorSets[i]);
        }


        RACBuffer UBOBuffer{
            racDevice, sizeof(UBO), MAX_FRAMES_IN_FLIGHT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, racDevice.properties.limits.minUniformBufferOffsetAlignment, };
        UBOBuffer.map();

        //camera.setViewDirection(glm::racc3(0.f), glm::racc3(0.5, 0.f, 1.f));
        camera.setViewTarget(glm::vec3(0.f, 5.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

        viewerObject = RACGameObject::createGameObject();
        keyboardCameraController = KeyboardCameraController();
        mouseCameraController = MouseCameraController();
    }

    started = true;
}
void Example::MyApplication::onDisconn() {
    connected = false;
    std::cout << "disc" << std::endl;
}

void Example::MyApplication::onUpdate(float totalTime, float deltaTime, std::set<int>& keysPressed, std::set<int>& keysReleased, std::set<int>& keysHeld) {
    //std::cout << "upd" << std::endl;
    gameObjects[0].transform.translation = glm::vec3(0.8 * glm::sin(totalTime), 0.6f, 0.8 * glm::cos(totalTime) + 2.5f);
    gameObjects[0].transform.rotation.y += deltaTime * 2.5f;

    keyboardCameraController.moveInPlaneXZ(keysHeld, deltaTime, viewerObject, totalTime);

    glm::vec2 toMove{ 0 };
    const double interpolation_ratio = 0.7;
    toMove.x = std::abs(mouseMoveSinceLastFrame.x) > 0.005 ? mouseMoveSinceLastFrame.x * interpolation_ratio : mouseMoveSinceLastFrame.x;
    toMove.y = std::abs(mouseMoveSinceLastFrame.y) > 0.005 ? mouseMoveSinceLastFrame.y * interpolation_ratio : mouseMoveSinceLastFrame.y;

    mouseCameraController.control(toMove.x, toMove.y, deltaTime, viewerObject);
    //mouseCameraController.control(mouseMoveSinceLastFrame.x, mouseMoveSinceLastFrame.y, deltaTime, viewerObject);

    camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);
    mouseMoveSinceLastFrame -= toMove;
   // mouseMoveSinceLastFrame = glm::vec2(0, 0);
}
void Example::MyApplication::onDraw(float totalTime, float deltaTime) {

    float aspect = racRenderer.getAspectRatio();
    camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);
    if (auto commandBuffer = racRenderer.beginFrame()) {
        int frameIndex = racRenderer.getFrameIndex();
        FrameInfo frameInfo{ frameIndex, deltaTime, commandBuffer, camera, globalDescriptorSets[frameIndex] };
        // update
        UBO ubo{};
        ubo.projectionView = camera.getProjection() * camera.getView();
        uboBuffers[frameIndex]->writeToBuffer(&ubo);
        uboBuffers[frameIndex]->flush();

        //rendering
        racRenderer.beginSwapChainRenderPass(commandBuffer);
        renderSystem.renderGameObjects(frameInfo, gameObjects);
        racRenderer.endSwapChainRenderPass(commandBuffer);
        racRenderer.endFrame();
    }
}

void Example::MyApplication::onKeyPress(int key, int mods, int scancode) {
    if (key == GLFW_KEY_R) {
        std::cout << "R PRESSED";
        viewerObject.transform.rotation = glm::vec3(0.f, 0.f, 0.f);
        viewerObject.transform.translation = glm::vec3(0.f, 0.f, 0.f);
    }

}
void Example::MyApplication::onKeyRelease(int key, int mods, int scancode) {

}
void Example::MyApplication::onMouseMove(double dX, double dY, double x, double y) {
        mouseMoveSinceLastFrame.x += dX / WIDTH;
        mouseMoveSinceLastFrame.y += dY / HEIGHT;
}
void Example::MyApplication::onMousePress(double x, double y, int button, int mods) {

}
void Example::MyApplication::onMouseRelease(double x, double y, int button, int mods) {

}
void Example::MyApplication::onWindowEvent(int x, int y) {

}


void Example::MyApplication::loadGameObjects() {
    std::shared_ptr<RACModel> racModel = RACModel::createModelFromFile(pid, racDevice, "assets/models/Scaniverse.obj");
    std::shared_ptr<RACTexture> racTexture = RACTexture::createTextureFromFile(pid, racDevice, "assets/textures/Scaniverse.jpg");

    std::shared_ptr<RACModel> planeModel = RACModel::createModelFromFile(pid, racDevice, "assets/models/plane.obj");
    std::shared_ptr<RACTexture> planeTexture = RACTexture::createTextureFromFile(pid, racDevice, "assets/textures/plane.jpg");

    auto obj = RACGameObject::createGameObject();
    obj.model = racModel;
    obj.texture = racTexture;
    obj.transform.translation = { 0.0f, 0.0f, 2.5f };
    obj.transform.scale = { 1.5f, 1.5f, 1.5f };
    obj.transform.rotation = { 3.14f, 0.0f, 0.0f };

    auto plane = RACGameObject::createGameObject();
    plane.model = planeModel;
    plane.texture = planeTexture;
    plane.transform.translation = { 0.0f, 0.35f, 2.5f };
    plane.transform.scale = { 2.f, 2.f, 2.f };
    plane.transform.rotation = { 1.57f, 0.0f, 0.0f };

    gameObjects.push_back(std::move(obj));
    gameObjects.push_back(std::move(plane));
}
