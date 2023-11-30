#pragma once

#include "RAC_device.hpp"

#include <string>
#include <vector>

namespace RAC {
	struct PipelineConfigInfo {
		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		std::vector<VkDynamicState> dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo dynamicStateInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
		PipelineConfigInfo() = default;
	private:
		PipelineConfigInfo(const PipelineConfigInfo&);
		PipelineConfigInfo& operator=(const PipelineConfigInfo&);
	};

	class RACPipeline {
	public:
		RACPipeline(RACDeviceBase& device, const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo);
		~RACPipeline();
		static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
		void bind(VkCommandBuffer commandBuffer);
	private:
		RACDeviceBase& racDevice;
		VkPipeline graphicsPipeline;
		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;
		static std::vector<char> readFile(const std::string& fpath);

		void createShaderModule(const std::vector<char>& shader, VkShaderModule* shaderModule);
		
		void createPipeline(const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo);
		RACPipeline(const RACPipeline&);
		RACPipeline& operator=(const RACPipeline&);
	};


}