#pragma once
#include <string>
#include <memory>
#include <stb_image.h>


#include "BFE_buffer.hpp"
namespace BFE {
    class BFEImage {
    public:
        struct Builder {
            VkImage image;
            stbi_uc* pixels;
            VkDeviceSize imageSize;
            int imgWidth, imgHeight, imgChannels;
            void loadImage(const std::string& fpath);
            void loadImage(const size_t size, unsigned char* data, const size_t width, const size_t height, const size_t channels = 4);
        };

        void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
        ~BFEImage();
        BFEImage(BFEDevice& device, Builder& builder);
        BFEImage(BFEDevice& device, Builder& builder, VkImageUsageFlags usage, VkImageLayout finalLayout);
        VkImage image;
        VkImageLayout layout;
        VkDeviceMemory imageMemory;
        static std::unique_ptr<BFEImage> createImageFromFile(BFEDevice& device, const std::string& fpath);
        static std::unique_ptr<BFEImage> createImageFromBuffer(BFEDevice& device, unsigned char* buffer, int width, int height, int channels = 3);
        unsigned int imgWidth, imgHeight, imgChannels;
        static void transitionVKImageLayout(BFEDevice& device, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

    private:
        BFEDevice& bfeDevice;
        std::unique_ptr<BFEBuffer> imageBuffer;

        BFEImage(const BFEImage& texture);
        BFEImage& operator=(const BFEImage&);

        VkCommandBuffer beginSingleTimeCommands();
        static VkCommandBuffer beginSingleTimeCommands(BFEDevice& device);
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);
        static void endSingleTimeCommands(VkCommandBuffer commandBuffer, BFEDevice& device);
        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    };
}