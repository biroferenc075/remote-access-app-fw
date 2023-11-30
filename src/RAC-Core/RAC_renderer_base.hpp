#pragma once

#include "RAC_window.hpp"
#include "RAC_device.hpp"
#include "RAC_swap_chain.hpp"
#include "RAC_model.hpp"
#include <memory>
#include <vector>
namespace RAC {
	class RACRendererBase {
	public:
		RACRendererBase(size_t pid, RACWindowBase& window, RACDeviceBase& device);
		~RACRendererBase();

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
		RACWindowBase& racWindow;
		RACDeviceBase& racDevice;

		std::vector<VkCommandBuffer> commandBuffers;
		RACRendererBase(const RACRendererBase&);
		RACRendererBase& operator=(const RACRendererBase&);
		uint32_t currentImageIndex{ 0 };
		int currentFrameIndex{ 0 };
		bool isFrameStarted{ false };
	};
}