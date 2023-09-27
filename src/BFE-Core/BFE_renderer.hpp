#pragma once

#include "BFE_renderer_base.hpp"
#include "BFE_window.hpp"
#include "BFE_device.hpp"
#include "BFE_swap_chain.hpp"
#include "BFE_model.hpp"
#include <memory>
#include <vector>
namespace BFE {
		class BFERenderer : public BFERendererBase {
	public:
		BFERenderer(size_t pid, BFEWindow& window, BFEDevice& device);
		~BFERenderer();

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
		VkImage getCurrentImage() const { return bfeSwapChain->getImage(currentImageIndex); }
		VkRenderPass getSwapChainRenderPass() const { return bfeSwapChain->getRenderPass(); }
		float getAspectRatio() const {
			return bfeSwapChain->extentAspectRatio();
		}
		uint32_t swapChainWidth() { return bfeSwapChain->width(); }
		uint32_t swapChainHeight() { return bfeSwapChain->height(); }
	private:
		void recreateSwapChain();
		std::unique_ptr<BFESwapChain> bfeSwapChain;
	};
}