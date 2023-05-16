#include "BFE_renderer.hpp"
#include <stdexcept>
#include <array>
#include <iostream>


namespace BFE {
	BFERenderer::BFERenderer(size_t pid, BFEWindow& window, BFEDevice& device) : bfeWindow{ window }, bfeDevice{ device }, pid(pid) {
		recreateSwapChain();
		createCommandBuffers();
	}

	BFERenderer::~BFERenderer() {
		freeCommandBuffers();
	}


	void BFERenderer::recreateSwapChain() {
		auto extent = bfeWindow.getExtent();

		while (extent.width == 0 || extent.height == 0) {
			extent = bfeWindow.getExtent();
			glfwWaitEvents();
		}

		//TODO mutex with transfers ?
		vkDeviceWaitIdle(bfeDevice.device());
		if (bfeSwapChain == nullptr)
		{
			bfeSwapChain = std::make_unique<BFESwapChain>(bfeDevice, extent);
		}
		else {
			std::shared_ptr<BFESwapChain> oldSwapChain = std::move(bfeSwapChain);
			bfeSwapChain = std::make_unique<BFESwapChain>(bfeDevice, extent, oldSwapChain);
			if (!oldSwapChain->compareSwapFormats(*bfeSwapChain.get())) {
				throw std::runtime_error("Swap chain image(or depth) format has changed!");
			}
		
		}

	}


	void BFERenderer::createCommandBuffers() {
		commandBuffers.resize(BFESwapChain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};

		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = bfeDevice.getCommandPool(pid);
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(bfeDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void BFERenderer::freeCommandBuffers() {
		vkFreeCommandBuffers(bfeDevice.device(), bfeDevice.getCommandPool(pid), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
		commandBuffers.clear();
	}

	VkCommandBuffer BFERenderer::beginFrame() {
		auto result = bfeSwapChain->acquireNextImage(&currentImageIndex);

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
	void BFERenderer::endFrame() {
		auto commandBuffer = getCurrentCommandBuffer();

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}

		auto result = bfeSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || bfeWindow.wasWindowResized()) {
			bfeWindow.resetWindowResizedFlag();
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % BFESwapChain::MAX_FRAMES_IN_FLIGHT;
	}
	void BFERenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = bfeSwapChain->getRenderPass();
		renderPassInfo.framebuffer = bfeSwapChain->getFrameBuffer(currentImageIndex);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = bfeSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(bfeSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(bfeSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, bfeSwapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	}
	void BFERenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {

		vkCmdEndRenderPass(commandBuffer);

	}

}