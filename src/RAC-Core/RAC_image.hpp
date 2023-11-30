#pragma once
#include <string>
#include <memory>
#include <stb_image.h>


#include "RAC_buffer.hpp"
namespace RAC {
    class RACImage {
    public:
        struct Builder {
            VkImage image;
            stbi_uc* pixels;
            VkDeviceSize imageSize;
            int imgWidth, imgHeight, imgChannels;
            void loadImage(const std::string& fpath);
            void loadImage(const size_t size, unsigned char* data, const size_t width, const size_t height, const size_t channels = 4);
        };

        void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, RACDeviceBase& racDevice);
        ~RACImage();
        RACImage(size_t pid, RACDeviceBase& device, Builder& builder);
        RACImage(size_t pid, RACDeviceBase& device, Builder& builder, VkImageUsageFlags usage, VkImageLayout finalLayout);
        VkImage image;
        VkImageLayout layout;
        VkDeviceMemory imageMemory;
        static std::unique_ptr<RACImage> createImageFromFile(size_t pid, RACDeviceBase& device, const std::string& fpath);
        static std::unique_ptr<RACImage> createImageFromBuffer(size_t pid, RACDeviceBase& device, unsigned char* buffer, int width, int height, int channels = 3);
        unsigned int imgWidth, imgHeight, imgChannels;
        static void transitionVKImageLayout(size_t pid, VkQueue queue, RACDeviceBase& device, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
        size_t pid;
    private:
        
        RACDeviceBase& racDevice;
        //std::unique_ptr<RACBuffer> imageBuffer;

        RACImage(const RACImage& texture);
        RACImage& operator=(const RACImage&);

        VkCommandBuffer beginSingleTimeCommands();
        static VkCommandBuffer beginSingleTimeCommands(size_t pid, RACDeviceBase& device);
        void endSingleTimeCommands(VkQueue queue, VkCommandBuffer commandBuffer);
        static void endSingleTimeCommands(size_t pid, VkQueue queue, VkCommandBuffer commandBuffer, RACDeviceBase& device);
        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        void transitionImageLayout(VkQueue queue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    };
}