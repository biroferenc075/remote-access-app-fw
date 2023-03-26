#pragma once

#include "BFE_device.hpp"

#include <string>
#include <vector>

namespace BFE {
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

	class BFEPipeline {
	public:
		BFEPipeline(BFEDevice& device, const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo);
		~BFEPipeline();
		static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
		void bind(VkCommandBuffer commandBuffer);
	private:
		BFEDevice& bfeDevice;
		VkPipeline graphicsPipeline;
		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;
		static std::vector<char> readFile(const std::string& fpath);

		void createShaderModule(const std::vector<char>& shader, VkShaderModule* shaderModule);
		
		void createPipeline(const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo);
		BFEPipeline(const BFEPipeline&);
		BFEPipeline& operator=(const BFEPipeline&);
	};


}