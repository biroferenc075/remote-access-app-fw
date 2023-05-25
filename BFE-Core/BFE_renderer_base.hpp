#pragma once

#include "BFE_window.hpp"
#include "BFE_device.hpp"
#include "BFE_swap_chain.hpp"
#include "BFE_model.hpp"
#include <memory>
#include <vector>
namespace BFE {
	class BFERendererBase {
	public:
		BFERendererBase(size_t pid, BFEWindowBase& window, BFEDeviceBase& device);
		~BFERendererBase();

		virtual VkCommandBuffer beginFrame() = 0;
		virtual void endFrame() = 0;
		virtual void beginSwapChainRenderPass(VkCommandBuffer commandBuffer) = 0;
		virtual void endSwapChainRenderPass(VkCommandBuffer commandBuffer) = 0;
		virtual VkImage getCurrentImage() const = 0;
		virtual VkRenderPass getSwapChainRenderPass() const = 0;
		virtual float getAspectRatio() const = 0;
		virtual uint32_t swapChainWidth() = 0;
		virtual uint32_t swapChainHeight() = 0;
		bool isFrameinProgress() const { return isFrameStarted; }
		VkCommandBuffer getCurrentCommandBuffer() const { return commandBuffers[currentFrameIndex]; }
		int getFrameIndex() const {
			return currentFrameIndex;
		}
		size_t pid;
	protected:
		virtual void createCommandBuffers();
		virtual void freeCommandBuffers();
		BFEWindowBase& bfeWindow;
		BFEDeviceBase& bfeDevice;

		std::vector<VkCommandBuffer> commandBuffers;
		BFERendererBase(const BFERendererBase&);
		BFERendererBase& operator=(const BFERendererBase&);
		uint32_t currentImageIndex;
		int currentFrameIndex{ 0 };
		bool isFrameStarted{ false };
	};
}