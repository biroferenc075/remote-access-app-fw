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

#include <boost/asio.hpp>
#include <boost/bind/placeholders.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

using namespace sc;

DisplayModule::DisplayModule(boost::asio::io_context& io_context, bool& readyFlag, boost::condition_variable& readyCond, boost::mutex& mut) 
    : io_context_{ io_context}, timer(boost::asio::steady_timer(io_context)) , readyFlag(readyFlag), readyCond(readyCond), mut(mut) {
    std::cout << "displ ctor\n";
    timer.expires_after(dur);
}
DisplayModule::~DisplayModule() {}

const std::string& fpath = "image.png";

void DisplayModule::handler(const boost::system::error_code& error, BFERenderer* bfeRenderer) {
    if (!error)
    {
        std::cout << "displ handl\n";
        BFEImage* img;
        if (imageQueue.pop(img)) {
            auto buffer = bfeRenderer->beginFrame();
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
            imgCopy.extent = VkExtent3D{ img->imgWidth, img->imgHeight, 1 };


            vkCmdCopyImage(buffer, img->image, img->layout, bfeRenderer->getCurrentImage(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 1, &imgCopy);
            bfeRenderer->endFrame();

            //TODO free image? sync with display?
        }
    }
    timer.expires_after(dur);
}


void DisplayModule::run() {
    std::cout << "displ run\n";
    auto handlerfunc = boost::bind(&DisplayModule::handler, this, boost::placeholders::_1, &bfeRenderer);
    
    boost::lock_guard<boost::mutex> lock(mut);

    std::cout << "displ continue run\n";
    readyFlag = true;
    timer.async_wait(handlerfunc);
    //readyCond.notify_all();

    while (!bfeWindow.shouldClose()) {
        glfwPollEvents();
        timer.async_wait(handlerfunc);
    }

    vkDeviceWaitIdle(bfeDevice.device());
}

void DisplayModule::submitToQueue(BFEImage* img) {
    imageQueue.push(img); // TODO look into changing to bounded_push
    std::cout << "displ queuesubm\n";
}