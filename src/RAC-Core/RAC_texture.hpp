#pragma once
#include <string>
#include <memory>
#include <stb_image.h>


#include "RAC_buffer.hpp"

namespace RAC {
    class RACTexture {
    public:
        struct Builder {
            VkImage textureImage;
            stbi_uc* pixels;
            VkDeviceSize imageSize;
            int texWidth, texHeight, texChannels;
            void loadTexture(const std::string& fpath);
        };

        void createTextureImage(RACDeviceBase& device, Builder& builder);
        void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
        ~RACTexture();


        RACTexture(size_t pid, RACDeviceBase& device, Builder& builder);
        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;
        VkSampler textureSampler;
        static std::unique_ptr<RACTexture> createTextureFromFile(size_t pid, RACDeviceBase& device, const std::string& fpath);
        size_t pid;
    private:

        RACDeviceBase& racDevice;
        std::unique_ptr<RACBuffer> textureBuffer;
        

        RACTexture(const RACTexture& texture);
        RACTexture& operator=(const RACTexture&);

        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);
        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        void createTextureImageView();
        VkImageView createImageView(VkImage image, VkFormat format);
        void createTextureSampler();

    };
}