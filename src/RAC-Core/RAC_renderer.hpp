#pragma once

#include "RAC_renderer_base.hpp"
#include "RAC_window.hpp"
#include "RAC_device.hpp"
#include "RAC_swap_chain.hpp"
#include "RAC_model.hpp"
#include <memory>
#include <vector>
namespace RAC {
		class RACRenderer : public RACRendererBase {
	public:
		RACRenderer(size_t pid, RACWindow& window, RACDevice& device);
		~RACRenderer();

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
		VkImage getCurrentImage() const { return racSwapChain->getImage(currentImageIndex); }
		VkRenderPass getSwapChainRenderPass() const { return racSwapChain->getRenderPass(); }
		float getAspectRatio() const {
			return racSwapChain->extentAspectRatio();
		}
		uint32_t swapChainWidth() { return racSwapChain->width(); }
		uint32_t swapChainHeight() { return racSwapChain->height(); }
	private:
		void recreateSwapChain();
		std::unique_ptr<RACSwapChain> racSwapChain;
	};
}