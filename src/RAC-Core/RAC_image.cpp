#include "RAC_image.hpp"
#include "RAC_buffer.hpp"

#include <stdexcept>
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#include <stb_image.h>
#include <iostream>
namespace RAC {
    RACImage::RACImage(size_t pid, RACDeviceBase& device, Builder& builder) : racDevice{ device }, pid(pid) {
        RACBuffer stagingBuffer = RACBuffer{ device, builder.imageSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)builder.pixels);
        //imageBuffer = std::make_unique<RACBuffer>(racDevice, builder.imageSize, 1, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        //racDevice.copyBuffer(stagingBuffer.getBuffer(), imageBuffer.get()->getBuffer(), builder.imageSize);

        createImage(builder.imgWidth, builder.imgHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory, device);
        transitionImageLayout(device.transferQueue(), image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        copyBufferToImage(stagingBuffer.getBuffer(), image, static_cast<uint32_t>(builder.imgWidth), static_cast<uint32_t>(builder.imgHeight));
        transitionImageLayout(device.transferQueue(), image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

        imgWidth = builder.imgWidth;
        imgHeight = builder.imgHeight;
        imgChannels = builder.imgChannels;

        stbi_image_free(builder.pixels);
    }
    RACImage::RACImage(size_t pid, RACDeviceBase& device, Builder& builder, VkImageUsageFlags usage, VkImageLayout finalLayout) : racDevice{ device }, pid(pid) {
        RACBuffer stagingBuffer = RACBuffer{ device, builder.imageSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, };
        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)builder.pixels);
        //imageBuffer = std::make_unique<RACBuffer>(racDevice, builder.imageSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        //racDevice.copyBuffer(stagingBuffer.getBuffer(), imageBuffer.get()->getBuffer(), builder.imageSize);

        createImage(builder.imgWidth, builder.imgHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory, device);
        transitionImageLayout(device.transferQueue(), image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        copyBufferToImage(stagingBuffer.getBuffer(), image, static_cast<uint32_t>(builder.imgWidth), static_cast<uint32_t>(builder.imgHeight));
        transitionImageLayout(device.transferQueue(), image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, finalLayout);

        imgWidth = builder.imgWidth;
        imgHeight = builder.imgHeight;
        imgChannels = builder.imgChannels;

        stbi_image_free(builder.pixels);
    }
    RACImage::~RACImage() {
        vkDestroyImage(racDevice.device(), image, nullptr);
        vkFreeMemory(racDevice.device(), imageMemory, nullptr);
    }

    void RACImage::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, RACDeviceBase& racDevice) {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        
        imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT; //VK_SHARING_MODE_EXCLUSIVE 
        imageInfo.queueFamilyIndexCount = 2; 
        imageInfo.pQueueFamilyIndices = racDevice.queueFamilyIndices;

        if (vkCreateImage(racDevice.device(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(racDevice.device(), image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = racDevice.findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(racDevice.device(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(racDevice.device(), image, imageMemory, 0);
    }

    VkCommandBuffer RACImage::beginSingleTimeCommands() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = racDevice.getCommandPool(pid);
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(racDevice.device(), &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    VkCommandBuffer RACImage::beginSingleTimeCommands(size_t pid, RACDeviceBase& device) {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = device.getCommandPool(pid);
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device.device(), &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void RACImage::endSingleTimeCommands(VkQueue queue, VkCommandBuffer commandBuffer) {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);
        //vkDeviceWaitIdle TODO

        vkFreeCommandBuffers(racDevice.device(), racDevice.getCommandPool(pid), 1, &commandBuffer);
    }

    void RACImage::endSingleTimeCommands(size_t pid, VkQueue queue, VkCommandBuffer commandBuffer, RACDeviceBase& device) {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);

        vkFreeCommandBuffers(device.device(), device.getCommandPool(pid), 1, &commandBuffer);
    }

    void RACImage::transitionImageLayout(VkQueue queue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
        transitionVKImageLayout(pid, queue, racDevice, image, format, oldLayout, newLayout);
        layout = newLayout;
    }

    void RACImage::transitionVKImageLayout(size_t pid, VkQueue queue, RACDeviceBase& device, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(pid, device);

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;

        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        endSingleTimeCommands(pid, queue, commandBuffer, device);
    }

    void RACImage::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        endSingleTimeCommands(racDevice.transferQueue(), commandBuffer);
    }
   

    std::unique_ptr<RACImage> RACImage::createImageFromFile(size_t pid, RACDeviceBase& device, const std::string& fpath) {
        Builder builder;
        builder.loadImage(fpath);
        return std::make_unique<RACImage>(pid, device, builder);
    }
    std::unique_ptr<RACImage> RACImage::createImageFromBuffer(size_t pid, RACDeviceBase& device, unsigned char* buffer, int width, int height, int channels) {
        Builder builder;
        builder.pixels = buffer;
        builder.imgWidth = width;
        builder.imgHeight = height;
        builder.imgChannels = channels;
        builder.imageSize = width * height * 4;
        return std::make_unique<RACImage>(pid, device, builder);
    }


    void RACImage::Builder::loadImage(const std::string& fpath) {
        pixels = stbi_load(fpath.c_str(), &imgWidth, &imgHeight, &imgChannels, STBI_rgb_alpha);
        imageSize = imgWidth * imgHeight * 4;

        if (!pixels) {
            throw std::runtime_error("failed to load image!");
        }
    }

    void RACImage::Builder::loadImage(const size_t size, unsigned char* data, const size_t width, const size_t height, const size_t channels) {
        pixels = data;
        imageSize = size;
        imgWidth = width;
        imgHeight = height;
        imgChannels = channels;
    }
}