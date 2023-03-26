#pragma once

#include "BFE_camera.hpp"

#include <vulkan/vulkan.h>

namespace BFE {
	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		BFECamera& camera;
		VkDescriptorSet globalDescriptorSet;
	};
}