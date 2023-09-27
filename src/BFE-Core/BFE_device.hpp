#pragma once

#include "BFE_device_base.hpp"
#include "BFE_window.hpp"

#include <string>
#include <vector>

namespace BFE {


class BFEDevice : public BFEDeviceBase {
 public:
#ifdef NDEBUG
  const bool enableValidationLayers = false;
#else
  const bool enableValidationLayers = true;
#endif
  VkQueue presentQueue() { return presentQueue_; }
  BFEDevice(BFEWindow &window);
  ~BFEDevice();

  BFEDevice(const BFEDevice &) = delete;
  void operator=(const BFEDevice &) = delete;
  BFEDevice(BFEDevice &&) = delete;
  BFEDevice &operator=(BFEDevice &&) = delete;

  VkSurfaceKHR surface() { return surface_; }

  SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice); }
 protected:
  void createLogicalDevice();
  VkQueue presentQueue_;
  void createInstance();
  void createSurface();
  bool isDeviceSuitable(VkPhysicalDevice device);
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
  std::vector<const char*> getRequiredExtensions();
  void hasGflwRequiredInstanceExtensions();
  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
  VkSurfaceKHR surface_;
 
  std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
  std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};

}