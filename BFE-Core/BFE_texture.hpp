#pragma once
#include <string>
#include <memory>
#include <stb_image.h>


#include "BFE_buffer.hpp"

namespace BFE {
    class BFETexture {
    public:
        struct Builder {
            VkImage textureImage;
            stbi_uc* pixels;
            VkDeviceSize imageSize;
            int texWidth, texHeight, texChannels;
            void loadTexture(const std::string& fpath);
        };

        void createTextureImage(BFEDevice& device, Builder& builder);
        void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
        ~BFETexture();


        BFETexture(BFEDevice& device, Builder& builder);
        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;
        VkSampler textureSampler;
        static std::unique_ptr<BFETexture> createTextureFromFile(BFEDevice& device, const std::string& fpath);
    private:

        BFEDevice& bfeDevice;
        std::unique_ptr<BFEBuffer> textureBuffer;
        

        BFETexture(const BFETexture& texture);
        BFETexture& operator=(const BFETexture&);

        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);
        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        void createTextureImageView();
        VkImageView createImageView(VkImage image, VkFormat format);
        void createTextureSampler();

    };
}