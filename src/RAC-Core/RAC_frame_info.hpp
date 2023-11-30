#pragma once

#include "RAC_camera.hpp"

#include <vulkan/vulkan.h>

namespace RAC {
	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		RACCamera& camera;
		VkDescriptorSet globalDescriptorSet;
	};
}