#include "displayMod.hpp"
#include <iostream>

#include "../RAC-Core/RAC_image.hpp"
#include "../RAC-Core/RAC_renderer.hpp"
#include "../RAC-Core/RAC_buffer.hpp"

#include <stdexcept>
#include <array>
#include <numeric>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <string>

#include <boost/asio.hpp>
#include <boost/bind/placeholders.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

using namespace rc;

DisplayModule::DisplayModule(boost::asio::io_context& io_context, bool& readyFlag, boost::condition_variable& readyCond, boost::mutex& mut, RACWindow& racWindow, RACDevice& racDevice)
    : io_context_{ io_context}, timer(boost::asio::steady_timer(io_context)), everyoneReady(readyFlag), readyCond(readyCond), mut(mut), racWindow(racWindow), racDevice(racDevice), pid(racDevice.allocateCommandPool()), racRenderer(pid, racWindow, racDevice) {

    racRenderer.pid = pid;
    }
DisplayModule::~DisplayModule() {}


void DisplayModule::handler(const boost::system::error_code& error, RACRenderer* racRenderer) {
    if (!error)
    {
        RACImage* img;

        VkImage currImg = racRenderer->getCurrentImage();
        if (imageQueue.pop(img)) {
            auto buffer = racRenderer->beginFrame();

            VkImageCopy imgCopy{};

            VkImageSubresourceLayers srcSubres{};
            srcSubres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            srcSubres.baseArrayLayer = 0;
            srcSubres.layerCount = 1;
            srcSubres.mipLevel = 0;

            VkImageSubresourceLayers dstSubres{};
            dstSubres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            dstSubres.baseArrayLayer = 0;
            dstSubres.layerCount = 1;
            dstSubres.mipLevel = 0;

            imgCopy.srcSubresource = srcSubres;
            imgCopy.dstSubresource = dstSubres;
            imgCopy.srcOffset = VkOffset3D{ 0,0,0 };
            imgCopy.dstOffset = VkOffset3D{ 0,0,0 };
            uint32_t w = img->imgWidth < racRenderer->swapChainWidth() ? img->imgWidth : racRenderer->swapChainWidth();
            uint32_t h = img->imgHeight < racRenderer->swapChainHeight() ? img->imgHeight : racRenderer->swapChainHeight();
            imgCopy.extent = VkExtent3D{ w, h, 1};
            
            // transition layout
            VkImageMemoryBarrier barrier1{};
            barrier1.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier1.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            barrier1.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

            barrier1.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier1.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            barrier1.image = currImg;
            barrier1.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier1.subresourceRange.baseMipLevel = 0;
            barrier1.subresourceRange.levelCount = 1;
            barrier1.subresourceRange.baseArrayLayer = 0;
            barrier1.subresourceRange.layerCount = 1;
            VkPipelineStageFlags sourceStage1;
            VkPipelineStageFlags destinationStage1;

            barrier1.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
            barrier1.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage1 = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            destinationStage1 = VK_PIPELINE_STAGE_TRANSFER_BIT;
            
            vkCmdPipelineBarrier(
                buffer,
                sourceStage1, destinationStage1,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier1
            );
           
            vkCmdCopyImage(buffer, img->image, img->layout, currImg, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imgCopy);
                   
            // transition layout
            VkImageMemoryBarrier barrier2{};
            barrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier2.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier2.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            barrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            barrier2.image = currImg;
            barrier2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier2.subresourceRange.baseMipLevel = 0;
            barrier2.subresourceRange.levelCount = 1;
            barrier2.subresourceRange.baseArrayLayer = 0;
            barrier2.subresourceRange.layerCount = 1;
            VkPipelineStageFlags sourceStage2;
            VkPipelineStageFlags destinationStage2;

            barrier2.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier2.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

            sourceStage2 = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage2 = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

            vkCmdPipelineBarrier(
                buffer,
                sourceStage2, destinationStage2,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier2
            );
            racRenderer->endFrame();

            vkQueueWaitIdle(racDevice.graphicsQueue());
            delete img;
        }
        if (!ioerror) {
            timer.expires_after(dur);
            auto handlerfunc = boost::bind(&DisplayModule::handler, this, boost::placeholders::_1, racRenderer);
            timer.async_wait(handlerfunc);
        }     
    }
    else {
        static bool written = false;
        written = true;
        ioerror = true;
    }
}


void DisplayModule::run() {
    try {
        auto handlerfunc = boost::bind(&DisplayModule::handler, this, boost::placeholders::_1, &racRenderer);
        isReady = true;

        boost::unique_lock<boost::mutex> lock(mut);
        while (!everyoneReady)
        {
            readyCond.wait(lock);
        }

        lock.unlock();

        timer.expires_after(dur);

        timer.async_wait(handlerfunc);
        auto err = boost::system::error_code();

        while (!racWindow.shouldClose()) {
            glfwPollEvents();
            if (ioerror) {
                timer.cancel();
                break;
            }
            Sleep(5);
        }
        RACImage* img;

        while (imageQueue.pop(img)) {
            delete img;
        }
        timer.cancel();
        vkDeviceWaitIdle(racDevice.device());
    }
    catch (std::exception e) {
        std::cout << e.what();
    }
}

void DisplayModule::submitToQueue(RACImage* img) {
    imageQueue.push(img);
    img->pid = this->pid;
}