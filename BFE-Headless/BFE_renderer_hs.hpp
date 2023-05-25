#pragma once

#include "BFE_image.hpp"
#include "BFE_renderer_base.hpp"
#include "BFE_window_hs.hpp"
#include "BFE_device_hs.hpp"
#include "BFE_swap_chain.hpp"
#include "BFE_model.hpp"
#include "frame.hpp"
#include "boost/lockfree/queue.hpp"
#include <memory>
#include <vector>
namespace BFE {
	class BFERendererHS : public BFERendererBase {
	public:
		BFERendererHS(size_t pid, BFEWindowHS& window, BFEDeviceHS& device, boost::lockfree::queue<Frame*>& frameQueue);
		~BFERendererHS();

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
		VkImage getCurrentImage() const { return images[currentImageIndex]; }
		VkRenderPass getSwapChainRenderPass() const { return renderPass; }
		float getAspectRatio() const {
			return static_cast<float>(extent.width) / static_cast<float>(extent.height);
		}
		uint32_t swapChainWidth() { return extent.width; }
		uint32_t swapChainHeight() { return extent.height; }
		size_t imageCount() { return images.size(); }
	private:
		void createCommandBuffers();
		void freeCommandBuffers();
		void init();
		void createImages();
		void createImageViews();
		void createDepthResources();
		void createRenderPass();
		void createFramebuffers();
		void createSyncObjects();
		VkFormat findDepthFormat();

		VkRenderPass renderPass;
		VkFormat imageFormat;
		VkFormat depthFormat;
		VkExtent2D extent;
		VkCommandBuffer getCurrentCommandBufferTransfer() const { return commandBuffersTransfer[currentImageIndex]; }
		std::vector<VkCommandBuffer> commandBuffersTransfer;
		std::vector<VkFramebuffer> framebuffers;
		std::vector<VkImage> images;
		std::vector<VkDeviceMemory> imageMemorys;
		std::vector<VkImageView> imageViews;
		std::vector<VkImage> depthImages;
		std::vector<VkDeviceMemory> depthImageMemorys;
		std::vector<VkImageView> depthImageViews;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imagesInFlight;
		size_t currentFrame = 0;

		boost::lockfree::queue<Frame*> &frameQueue;
	};
}