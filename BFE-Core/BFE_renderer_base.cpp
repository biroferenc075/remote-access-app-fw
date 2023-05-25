#include "BFE_renderer_base.hpp"
#include "consts.hpp"

namespace BFE {
	
	BFERendererBase::BFERendererBase(size_t pid, BFEWindowBase& window, BFEDeviceBase& device) : pid(pid), bfeWindow(window), bfeDevice(device) {
		createCommandBuffers();
	}
	BFERendererBase::~BFERendererBase() {
		freeCommandBuffers();
	}
	void BFERendererBase::createCommandBuffers() {
		commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

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

	void BFERendererBase::freeCommandBuffers() {
		vkFreeCommandBuffers(bfeDevice.device(), bfeDevice.getCommandPool(pid), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
		commandBuffers.clear();
	}
}

