#pragma once

#include "BFE_window.hpp"
#include "BFE_device.hpp"
#include "BFE_swap_chain.hpp"
#include "BFE_model.hpp"
#include <memory>
#include <vector>
namespace BFE {
		class BFERenderer {
	public:
		BFERenderer(BFEWindow& window, BFEDevice& device);
		~BFERenderer();

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
		bool isFrameinProgress() const { return isFrameStarted; }
		VkCommandBuffer getCurrentCommandBuffer() const { return commandBuffers[currentFrameIndex]; }
		VkImage getCurrentImage() const { return bfeSwapChain->getImage(currentImageIndex); }
		VkRenderPass getSwapChainRenderPass() const { return bfeSwapChain->getRenderPass(); }
		float getAspectRatio() const {
			return bfeSwapChain->extentAspectRatio();
		}
		int getFrameIndex() const {
			return currentFrameIndex;
		}
	private:
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();
		BFEWindow& bfeWindow;
		BFEDevice& bfeDevice;
		std::unique_ptr<BFESwapChain> bfeSwapChain;
		std::vector<VkCommandBuffer> commandBuffers;
		BFERenderer(const BFERenderer&);
		BFERenderer& operator=(const BFERenderer&);
		uint32_t currentImageIndex;
		int currentFrameIndex{0};
		bool isFrameStarted{ false};
	};
}