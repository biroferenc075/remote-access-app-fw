#include "BFE_renderer_hs.hpp"
#include "consts.hpp"
#include <array>


#include "stb_image_write.h"
#include "stb_image.h"


namespace BFE {

	BFERendererHS::BFERendererHS(size_t pid, BFEWindowHS& window, BFEDeviceHS& device, boost::lockfree::queue<Frame*>& frameQueue) : BFERendererBase(pid, window, device), frameQueue(frameQueue) {
		init();
	}
	BFERendererHS::~BFERendererHS() {
		for (auto imageView : imageViews) {
			vkDestroyImageView(bfeDevice.device(), imageView, nullptr);
		}
		imageViews.clear();

		for (int i = 0; i < depthImages.size(); i++) {
			vkDestroyImageView(bfeDevice.device(), depthImageViews[i], nullptr);
			vkDestroyImage(bfeDevice.device(), depthImages[i], nullptr);
			vkFreeMemory(bfeDevice.device(), depthImageMemorys[i], nullptr);
		}

		for (auto framebuffer : framebuffers) {
			vkDestroyFramebuffer(bfeDevice.device(), framebuffer, nullptr);
		}

		vkDestroyRenderPass(bfeDevice.device(), renderPass, nullptr);

		// cleanup synchronization objects
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(bfeDevice.device(), renderFinishedSemaphores[i], nullptr);

			vkDestroyFence(bfeDevice.device(), inFlightFences[i], nullptr);
		}

		//freeCommandBuffers();
	}

	void BFERendererHS::init() {
		createCommandBuffers();
		extent = VkExtent2D{};
		extent.width = WIDTH;
		extent.height = HEIGHT;

		createImages();
		createImageViews();
		createRenderPass();
		createDepthResources();
		createFramebuffers();
		createSyncObjects();
	}

	VkCommandBuffer BFERendererHS::beginFrame() {
		vkWaitForFences(
			bfeDevice.device(),
			1,
			&inFlightFences[currentFrame],
			VK_TRUE,
			std::numeric_limits<uint64_t>::max());

		isFrameStarted = true;

		auto commandBuffer = getCurrentCommandBuffer();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		return commandBuffer;
	}
	void BFERendererHS::endFrame() {
		auto commandBuffer = getCurrentCommandBuffer();

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}

		if (imagesInFlight[currentImageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(bfeDevice.device(), 1, &imagesInFlight[currentImageIndex], VK_TRUE, UINT64_MAX);
		}
		imagesInFlight[currentImageIndex] = inFlightFences[currentFrame];

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		submitInfo.waitSemaphoreCount = 0;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[currentImageIndex];

		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(bfeDevice.device(), 1, &inFlightFences[currentFrame]);
		if (vkQueueSubmit(bfeDevice.graphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) !=
			VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		//
		VkImageCreateInfo imgCreateInfo{};

		imgCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imgCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imgCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		imgCreateInfo.extent.width = extent.width;
		imgCreateInfo.extent.height = extent.height;
		imgCreateInfo.extent.depth = 1;
		imgCreateInfo.arrayLayers = 1;
		imgCreateInfo.mipLevels = 1;
		imgCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imgCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imgCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
		imgCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		// Create the image
		VkImage dstImage;
		if (vkCreateImage(bfeDevice.device(), &imgCreateInfo, nullptr, &dstImage) != VK_SUCCESS) {
			throw std::runtime_error("cannot create destination image!");
		}
		// Create memory to back up the image
		VkMemoryRequirements memRequirements;
		VkMemoryAllocateInfo memAllocInfo{};
		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		VkDeviceMemory dstImageMemory;
		vkGetImageMemoryRequirements(bfeDevice.device(), dstImage, &memRequirements);
		memAllocInfo.allocationSize = memRequirements.size;
		// Memory must be host visible to copy from
		memAllocInfo.memoryTypeIndex = bfeDevice.findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		if (vkAllocateMemory(bfeDevice.device(), &memAllocInfo, nullptr, &dstImageMemory) != VK_SUCCESS) {
			throw std::runtime_error("cannot allocate memory for destination image!");
		}
		if (vkBindImageMemory(bfeDevice.device(), dstImage, dstImageMemory, 0) != VK_SUCCESS) {
			throw std::runtime_error("cannot bind memory for destination image!");
		}
		// Do the actual blit from the offscreen image to our host visible destination image


		VkCommandBuffer copyCmd = commandBuffersTransfer[currentImageIndex];

		VkCommandBufferBeginInfo cmdBufInfo = VkCommandBufferBeginInfo{};
		cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		if (vkBeginCommandBuffer(copyCmd, &cmdBufInfo) != VK_SUCCESS) {
			throw std::runtime_error("cannot begin transfer command buffer!");
		}

		// Transition destination image to transfer destination layout
		VkImageMemoryBarrier barrier1{};
		barrier1.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier1.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier1.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

		barrier1.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier1.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier1.image = dstImage;
		barrier1.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier1.subresourceRange.baseMipLevel = 0;
		barrier1.subresourceRange.levelCount = 1;
		barrier1.subresourceRange.baseArrayLayer = 0;
		barrier1.subresourceRange.layerCount = 1;
		VkPipelineStageFlags sourceStage1;
		VkPipelineStageFlags destinationStage1;

		barrier1.srcAccessMask = 0;
		barrier1.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage1 = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage1 = VK_PIPELINE_STAGE_TRANSFER_BIT;

		vkCmdPipelineBarrier(
			copyCmd,
			sourceStage1, destinationStage1,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier1
		);

		// image is already in VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, and does not need to be transitioned

		VkImageCopy imageCopyRegion{};
		imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageCopyRegion.srcSubresource.layerCount = 1;
		imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageCopyRegion.dstSubresource.layerCount = 1;
		imageCopyRegion.extent.width = extent.width;
		imageCopyRegion.extent.height = extent.height;
		imageCopyRegion.extent.depth = 1;

		vkCmdCopyImage(
			copyCmd,
			images[currentImageIndex], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&imageCopyRegion);

		// Transition destination image to general layout, which is the required layout for mapping the image memory later on
		VkImageMemoryBarrier barrier2{};
		barrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier2.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier2.newLayout = VK_IMAGE_LAYOUT_GENERAL;

		barrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier2.image = dstImage;
		barrier2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier2.subresourceRange.baseMipLevel = 0;
		barrier2.subresourceRange.levelCount = 1;
		barrier2.subresourceRange.baseArrayLayer = 0;
		barrier2.subresourceRange.layerCount = 1;
		VkPipelineStageFlags sourceStage2;
		VkPipelineStageFlags destinationStage2;

		barrier2.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier2.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

		sourceStage2 = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage2 = VK_PIPELINE_STAGE_TRANSFER_BIT;

		vkCmdPipelineBarrier(
			copyCmd,
			sourceStage2, destinationStage2,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier2
		);

		if (vkEndCommandBuffer(copyCmd) != VK_SUCCESS) {
			throw std::runtime_error("cannot finish recording transfer command buffer!");
		}
		
		VkSubmitInfo submitInfo2 = VkSubmitInfo{};
		submitInfo2.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo2.commandBufferCount = 1;
		submitInfo2.pCommandBuffers = &copyCmd;

		VkSemaphore waitSemaphores2[] = { renderFinishedSemaphores[currentFrame] };
		VkPipelineStageFlags waitStages2[] = { VK_PIPELINE_STAGE_TRANSFER_BIT };
		submitInfo2.waitSemaphoreCount = 1;
		submitInfo2.pWaitSemaphores = waitSemaphores2;
		submitInfo2.pWaitDstStageMask = waitStages2;
		submitInfo2.signalSemaphoreCount = 0;

		VkFenceCreateInfo fenceInfo = VkFenceCreateInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		VkFence fence;
		if (vkCreateFence(bfeDevice.device(), &fenceInfo, nullptr, &fence) != VK_SUCCESS) {
			throw std::runtime_error("cannot create fence for transfer!");
		}
		if (vkQueueSubmit(bfeDevice.transferQueue(), 1, &submitInfo2, fence) != VK_SUCCESS) {
			throw std::runtime_error("cannot submit work to transfer queue!");
		}
		vkQueueWaitIdle(bfeDevice.transferQueue()); //vkDeviceWaitIdle(bfeDevice.device()); //vkQueueWaitIdle(bfeDevice.transferQueue());
		vkWaitForFences(bfeDevice.device(), 1, &fence, VK_TRUE, UINT64_MAX);
		vkDestroyFence(bfeDevice.device(), fence, nullptr);


		// Get layout of the image (including row pitch)
		VkImageSubresource subResource{};
		subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		VkSubresourceLayout subResourceLayout;

		vkGetImageSubresourceLayout(bfeDevice.device(), dstImage, &subResource, &subResourceLayout);

		// Create frame object so we can start copying data to it
		
		const char* imagedata;

		// Map image memory so we can start copying from it
		vkMapMemory(bfeDevice.device(), dstImageMemory, 0, VK_WHOLE_SIZE, 0, (void**)&imagedata);
		imagedata += subResourceLayout.offset;
		
		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;

		Frame* fr = new Frame(extent.width * extent.height * 4);

		int s = 0;
		for (uint32_t y = 0; y < extent.height; y++) {
			unsigned char* row = (unsigned char*)imagedata;
			memcpy(fr->data + s, row, 4*extent.width);
			s += 4 * extent.width;
			/*for (int32_t x = 0; x < extent.width; x++) {
				memcpy(fr->data + s, row, 4);
				row+= 4;
			}*/
			imagedata += subResourceLayout.rowPitch;
		}

		if (!frameQueue.push(fr)) {
			delete fr;
		}

		vkUnmapMemory(bfeDevice.device(), dstImageMemory);
		vkFreeMemory(bfeDevice.device(), dstImageMemory, nullptr);
		vkDestroyImage(bfeDevice.device(), dstImage, nullptr);

		vkQueueWaitIdle(bfeDevice.transferQueue());
	}

	void BFERendererHS::createCommandBuffers() {
		commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		commandBuffersTransfer.resize(MAX_FRAMES_IN_FLIGHT);
		VkCommandBufferAllocateInfo allocInfo{};

		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = bfeDevice.getCommandPool(pid);
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		VkCommandBufferAllocateInfo allocInfo2{};

		allocInfo2.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo2.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo2.commandPool = bfeDevice.getCommandPool(pid);
		allocInfo2.commandBufferCount = static_cast<uint32_t>(commandBuffersTransfer.size());

		if (vkAllocateCommandBuffers(bfeDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}

		if (vkAllocateCommandBuffers(bfeDevice.device(), &allocInfo2, commandBuffersTransfer.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void BFERendererHS::freeCommandBuffers() {
		vkFreeCommandBuffers(bfeDevice.device(), bfeDevice.getCommandPool(pid), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
		vkFreeCommandBuffers(bfeDevice.device(), bfeDevice.getCommandPool(pid), static_cast<uint32_t>(commandBuffersTransfer.size()), commandBuffersTransfer.data());
		commandBuffers.clear();
		commandBuffersTransfer.clear();
	}

	void BFERendererHS::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = framebuffers[currentImageIndex];

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = extent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(extent.width);
		viewport.height = static_cast<float>(extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, extent };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}
	void BFERendererHS::endSwapChainRenderPass(VkCommandBuffer commandBuffer) { vkCmdEndRenderPass(commandBuffer); }
	
	void BFERendererHS::createImages() {
		imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
		images.resize(MAX_FRAMES_IN_FLIGHT);
		imageMemorys.resize(MAX_FRAMES_IN_FLIGHT);
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			auto imageInfo = VkImageCreateInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.format = imageFormat;
			imageInfo.extent.width = extent.width;
			imageInfo.extent.height = extent.height;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

			bfeDevice.createImageWithInfo(
				imageInfo,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				images[i],
				imageMemorys[i]);
		}
	}

	void BFERendererHS::createImageViews() {
		imageViews.resize(images.size());
		for (size_t i = 0; i < images.size(); i++) {
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = images[i];
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = imageFormat;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(bfeDevice.device(), &viewInfo, nullptr, &imageViews[i]) !=
				VK_SUCCESS) {
				throw std::runtime_error("failed to create texture image view!");
			}
		}
	}

	void BFERendererHS::createRenderPass() {
		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = findDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = imageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.srcAccessMask = 0;
		dependency.srcStageMask =
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstSubpass = 0;
		dependency.dstStageMask =
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask =
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(bfeDevice.device(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}

	void BFERendererHS::createFramebuffers() {
		framebuffers.resize(imageCount());
		for (size_t i = 0; i < imageCount(); i++) {
			std::array<VkImageView, 2> attachments = { imageViews[i], depthImageViews[i] };

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = extent.width;
			framebufferInfo.height = extent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(
				bfeDevice.device(),
				&framebufferInfo,
				nullptr,
				&framebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	void BFERendererHS::createDepthResources() {
		depthFormat = findDepthFormat();

		depthImages.resize(imageCount());
		depthImageMemorys.resize(imageCount());
		depthImageViews.resize(imageCount());

		for (int i = 0; i < depthImages.size(); i++) {
			VkImageCreateInfo imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = extent.width;
			imageInfo.extent.height = extent.height;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.format = depthFormat;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.flags = 0;

			bfeDevice.createImageWithInfo(
				imageInfo,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				depthImages[i],
				depthImageMemorys[i]);

			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = depthImages[i];
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = depthFormat;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(bfeDevice.device(), &viewInfo, nullptr, &depthImageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create texture image view!");
			}
		}
	}

	void BFERendererHS::createSyncObjects() {
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(bfeDevice.device(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) !=
				VK_SUCCESS ||
				vkCreateFence(bfeDevice.device(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
		}
	}

	VkFormat BFERendererHS::findDepthFormat() {
		return bfeDevice.findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}
}