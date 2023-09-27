#include "BFE_pipeline.hpp"
#include "BFE_model.hpp"

#include <fstream>
#include <stdexcept>

namespace BFE {
	BFEPipeline::BFEPipeline(BFEDeviceBase& device, const std::string& bfertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo) : bfeDevice{ device } {
		createPipeline(bfertPath, fragPath, configInfo);
	}

	BFEPipeline::~BFEPipeline() {
		vkDestroyShaderModule(bfeDevice.device(), vertShaderModule, nullptr);
		vkDestroyShaderModule(bfeDevice.device(), fragShaderModule, nullptr);
		vkDestroyPipeline(bfeDevice.device(), graphicsPipeline, nullptr);

	}


	std::vector<char> BFEPipeline::readFile(const std::string& fpath) {

		std::ifstream file(fpath, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file: " + fpath);
		}

		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();
		return buffer;
	}

	void BFEPipeline::createPipeline(const std::string& bfertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo) {
		auto bfertShader = readFile(bfertPath);
		auto fragShader = readFile(fragPath);
		
		createShaderModule(bfertShader, &vertShaderModule);
		createShaderModule(fragShader, &fragShaderModule);

		VkPipelineShaderStageCreateInfo shaderStages[2];
		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module = vertShaderModule;
		shaderStages[0].pName = "main";
		shaderStages[0].flags = 0;
		shaderStages[0].pNext = nullptr;
		shaderStages[0].pSpecializationInfo = nullptr;

		shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = fragShaderModule;
		shaderStages[1].pName = "main";
		shaderStages[1].flags = 0;
		shaderStages[1].pNext = nullptr;
		shaderStages[1].pSpecializationInfo = nullptr;

		auto bindingDescriptions = BFEModel::Vertex::getBindingDescriptions();
		auto attributeDescriptions = BFEModel::Vertex::getAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo bfertexInputInfo{};
		bfertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		bfertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		bfertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());;
		bfertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
		bfertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

		VkGraphicsPipelineCreateInfo pipelineinfo{};
		pipelineinfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineinfo.stageCount = 2;
		pipelineinfo.pStages = shaderStages;
		pipelineinfo.pVertexInputState = &bfertexInputInfo;
		pipelineinfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
		pipelineinfo.pViewportState = &configInfo.viewportInfo;
		pipelineinfo.pRasterizationState = &configInfo.rasterizationInfo;
		pipelineinfo.pMultisampleState = &configInfo.multisampleInfo;
		pipelineinfo.pColorBlendState = &configInfo.colorBlendInfo;
		pipelineinfo.pDepthStencilState = &configInfo.depthStencilInfo;
		pipelineinfo.pDynamicState = &configInfo.dynamicStateInfo;

		pipelineinfo.layout = configInfo.pipelineLayout;
		pipelineinfo.renderPass = configInfo.renderPass;
		pipelineinfo.subpass = configInfo.subpass;

		pipelineinfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineinfo.basePipelineIndex = -1;

		if (vkCreateGraphicsPipelines(bfeDevice.device(), VK_NULL_HANDLE, 1, &pipelineinfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
			}
	}

	void BFEPipeline::createShaderModule(const std::vector<char>& shader, VkShaderModule* shaderModule) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = shader.size();
			createInfo.pCode = reinterpret_cast<const uint32_t*>(shader.data());

			if (vkCreateShaderModule(bfeDevice.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
				throw std::runtime_error("failed to create shader module!");
			}
	}

	void BFEPipeline::defaultPipelineConfigInfo(PipelineConfigInfo& configInfo) {
		configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		configInfo.viewportInfo.viewportCount = 1;
		configInfo.viewportInfo.pViewports = nullptr;
		configInfo.viewportInfo.scissorCount = 1;
		configInfo.viewportInfo.pScissors = nullptr;

		
		configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
		configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
		configInfo.rasterizationInfo.lineWidth = 1.0f;
		configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
		configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
		//configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
		//configInfo.rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
		//configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional

		configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
		configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		//configInfo.multisampleInfo.minSampleShading = 1.0f;           // Optional
		//configInfo.multisampleInfo.pSampleMask = nullptr;             // Optional
		//configInfo.multisampleInfo.alphaToCobferageEnable = VK_FALSE;  // Optional
		//configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional

		configInfo.colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
		//configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
		//configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
		//configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
		//configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
		//configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
		//configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

		configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
		configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
		configInfo.colorBlendInfo.attachmentCount = 1;
		configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
		//configInfo.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
		//configInfo.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
		//configInfo.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
		//configInfo.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

		configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
		configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
		configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		//configInfo.depthStencilInfo.minDepthBounds = 0.0f;  // Optional
		//configInfo.depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
		//configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
		//configInfo.depthStencilInfo.front = {};  // Optional
		//configInfo.depthStencilInfo.back = {};   // Optional
		

		configInfo.dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
		configInfo.dynamicStateInfo.dynamicStateCount =
			static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
		configInfo.dynamicStateInfo.flags = 0;
	}

	void BFEPipeline::bind(VkCommandBuffer commandBuffer) {
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
	}
}