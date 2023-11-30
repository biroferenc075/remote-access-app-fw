#pragma once

#include "RAC_pipeline.hpp"
#include "RAC_device.hpp"
#include "RAC_model.hpp"
#include "RAC_gameobject.hpp"
#include "RAC_camera.hpp"
#include "RAC_frame_info.hpp"

#include <memory>
#include <vector>
namespace RAC {
	class RACRenderSystem {
	public:

		RACRenderSystem(RACDeviceBase& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~RACRenderSystem();

		void renderGameObjects(FrameInfo& frameInfo, std::vector<RACGameObject>& gameobjects);
	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);
		RACDeviceBase& racDevice;
		std::unique_ptr<RACPipeline> racPipeline;
		VkPipelineLayout pipelineLayout;
		RACRenderSystem(const RACRenderSystem&);
		RACRenderSystem& operator=(const RACRenderSystem&);
	};
}