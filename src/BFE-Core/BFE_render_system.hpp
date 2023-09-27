#pragma once

#include "BFE_pipeline.hpp"
#include "BFE_device.hpp"
#include "BFE_model.hpp"
#include "BFE_gameobject.hpp"
#include "BFE_camera.hpp"
#include "BFE_frame_info.hpp"

#include <memory>
#include <vector>
namespace BFE {
	class BFERenderSystem {
	public:

		BFERenderSystem(BFEDeviceBase& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~BFERenderSystem();

		void renderGameObjects(FrameInfo& frameInfo, std::vector<BFEGameObject>& gameobjects);
	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);
		BFEDeviceBase& bfeDevice;
		std::unique_ptr<BFEPipeline> bfePipeline;
		VkPipelineLayout pipelineLayout;
		BFERenderSystem(const BFERenderSystem&);
		BFERenderSystem& operator=(const BFERenderSystem&);
	};
}