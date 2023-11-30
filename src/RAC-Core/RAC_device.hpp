#pragma once

#include "RAC_device_base.hpp"
#include "RAC_window.hpp"

#include <string>
#include <vector>

namespace RAC {


class RACDevice : public RACDeviceBase {
 public:
#ifdef NDEBUG
  const bool enableValidationLayers = false;
#else
  const bool enableValidationLayers = true;
#endif
  VkQueue presentQueue() { return presentQueue_; }
  RACDevice(RACWindow &window);
  ~RACDevice();

  RACDevice(const RACDevice &) = delete;
  void operator=(const RACDevice &) = delete;
  RACDevice(RACDevice &&) = delete;
  RACDevice &operator=(RACDevice &&) = delete;

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