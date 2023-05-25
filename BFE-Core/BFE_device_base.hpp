#pragma once

#include "BFE_window.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace BFE {

    

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };


    struct QueueFamilyIndices {
        uint32_t presentFamily;
        uint32_t graphicsFamily;
        uint32_t transferFamily;
        bool presentFamilyHasValue = false;
        bool graphicsFamilyHasValue = false;
        bool transferFamilyHasValue = false;
        bool isComplete() { return graphicsFamilyHasValue && transferFamilyHasValue && presentFamilyHasValue; }
        bool isCompleteHeadless() { return graphicsFamilyHasValue && transferFamilyHasValue; }
    };

    class BFEDeviceBase {
    public:
#ifdef NDEBUG
        const bool enableValidationLayers = false;
#else
        const bool enableValidationLayers = true;
#endif

        size_t allocateCommandPool();
        VkCommandPool getCommandPool(size_t pid) { return commandPools.at(pid); }
        VkDevice device() { return device_; }
        VkQueue graphicsQueue() { return graphicsQueue_; }
       
        VkQueue transferQueue() { return transferQueue_; }

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        QueueFamilyIndices findPhysicalQueueFamilies() { return indices; }
        VkFormat findSupportedFormat(
            const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

        void createBuffer(
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer& buffer,
            VkDeviceMemory& bufferMemory);
        VkCommandBuffer beginSingleTimeCommands(size_t pid);
        void endSingleTimeCommands(size_t pid, VkCommandBuffer commandBuffer);
        void copyBuffer(size_t pid, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        void copyBufferToImage(size_t pid,
            VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

        void createImageWithInfo(
            const VkImageCreateInfo& imageInfo,
            VkMemoryPropertyFlags properties,
            VkImage& image,
            VkDeviceMemory& imageMemory);

        VkPhysicalDeviceProperties properties;

        uint32_t* queueFamilyIndices;
        VkInstance instance;
    protected:
        BFEDeviceBase(BFEWindowBase& window);
        virtual void createInstance();
        void setupDebugMessenger();
        void pickPhysicalDevice();
        virtual void createLogicalDevice();
        size_t createCommandPool();

        virtual bool isDeviceSuitable(VkPhysicalDevice device);
        virtual std::vector<const char*> getRequiredExtensions();
        bool checkValidationLayerSupport();
        virtual QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);


        VkDebugUtilsMessengerEXT debugMessenger;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        BFEWindowBase& window;
        std::vector<VkCommandPool> commandPools{};

        VkDevice device_;
        VkQueue graphicsQueue_;
        VkQueue transferQueue_;
        QueueFamilyIndices indices;

        const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
        const std::vector<const char*> deviceExtensions = {};
    };

}