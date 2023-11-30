#include "RAC_renderer.hpp"
#include <stdexcept>
#include <array>
#include <iostream>
#include "consts.hpp"

namespace RAC {
	RACRenderer::RACRenderer(size_t pid, RACWindow& window, RACDevice& device) : RACRendererBase(pid, window, device) {
		recreateSwapChain();
	}

	RACRenderer::~RACRenderer() {}


	void RACRenderer::recreateSwapChain() {
		auto extent = racWindow.getExtent();

		while (extent.width == 0 || extent.height == 0) {
			extent = racWindow.getExtent();
			glfwWaitEvents();
		}

		//TODO mutex with transfers ?
		vkDeviceWaitIdle(racDevice.device());
		if (racSwapChain == nullptr)
		{
			racSwapChain = std::make_unique<RACSwapChain>(dynamic_cast<RACDevice&>(racDevice), extent);
		}
		else {
			std::shared_ptr<RACSwapChain> oldSwapChain = std::move(racSwapChain);
			racSwapChain = std::make_unique<RACSwapChain>(dynamic_cast<RACDevice&>(racDevice), extent, oldSwapChain);
			if (!oldSwapChain->compareSwapFormats(*racSwapChain.get())) {
				throw std::runtime_error("Swap chain image(or depth) format has changed!");
			}
		
		}

	}

	VkCommandBuffer RACRenderer::beginFrame() {
		auto result = racSwapChain->acquireNextImage(&currentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			std::cout << "\nbeginframe vk out of date!\n";
			recreateSwapChain();
			return nullptr;
		}
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		isFrameStarted = true;

		auto commandBuffer = getCurrentCommandBuffer();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		return commandBuffer;
	}
	void RACRenderer::endFrame() {
		auto commandBuffer = getCurrentCommandBuffer();

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}

		auto result = racSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || racWindow.wasWindowResized()) {
			racWindow.resetWindowResizedFlag();
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
	}
	void RACRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = racSwapChain->getRenderPass();
		renderPassInfo.framebuffer = racSwapChain->getFrameBuffer(currentImageIndex);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = racSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(racSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(racSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, racSwapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	}
	void RACRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {

		vkCmdEndRenderPass(commandBuffer);

	}

}