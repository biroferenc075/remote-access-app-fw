#include "RAC_renderer_base.hpp"
#include "consts.hpp"

namespace RAC {
	
	RACRendererBase::RACRendererBase(size_t pid, RACWindowBase& window, RACDeviceBase& device) : pid(pid), racWindow(window), racDevice(device) {
		createCommandBuffers();
	}
	RACRendererBase::~RACRendererBase() {
		freeCommandBuffers();
	}
	void RACRendererBase::createCommandBuffers() {
		commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};

		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = racDevice.getCommandPool(pid);
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(racDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void RACRendererBase::freeCommandBuffers() {
		vkFreeCommandBuffers(racDevice.device(), racDevice.getCommandPool(pid), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
		commandBuffers.clear();
	}
}

