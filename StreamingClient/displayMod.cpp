#include "displayMod.hpp"
#include <iostream>

#include "BFE_image.hpp"
#include "BFE_renderer.hpp"
#include "BFE_buffer.hpp"

#include <stdexcept>
#include <array>
#include <numeric>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <string>
using namespace std;
using namespace sc;


DisplayModule::DisplayModule() {}
DisplayModule::~DisplayModule() {}


const std::string& fpath = "image.png";

void DisplayModule::run() {
    BFE::BFEWindow bfeWindow{ WIDTH, HEIGHT, "Vulkan" };
    BFE::BFEDevice bfeDevice{ bfeWindow };
    BFE::BFERenderer bfeRenderer{ bfeWindow, bfeDevice };
    BFE::BFEImage::Builder builder;
    builder.loadImage(fpath);
    BFE::BFEImage img{ bfeDevice, builder };

    while (!bfeWindow.shouldClose()) {
        glfwPollEvents();
        auto buffer = bfeRenderer.beginFrame();
        VkImageCopy imgCopy{};

        VkImageSubresourceLayers srcSubres{};
        srcSubres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        srcSubres.baseArrayLayer = 0;
        srcSubres.layerCount = 1;
        srcSubres.mipLevel = 0;


        VkImageSubresourceLayers dstSubres{};
        srcSubres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        srcSubres.baseArrayLayer = 0;
        srcSubres.layerCount = 1;
        srcSubres.mipLevel = 0;

        imgCopy.srcSubresource = srcSubres;
        imgCopy.dstSubresource = dstSubres;
        imgCopy.srcOffset = VkOffset3D{0,0,0};
        imgCopy.dstOffset = VkOffset3D{0,0,0};
        imgCopy.extent = VkExtent3D{img.imgWidth, img.imgHeight, 1};


        vkCmdCopyImage(buffer, img.image, img.layout, bfeRenderer.getCurrentImage(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 1, &imgCopy);
        bfeRenderer.endFrame();
    }

    vkDeviceWaitIdle(bfeDevice.device());
}