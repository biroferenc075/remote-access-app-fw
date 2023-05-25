#pragma once

#include "BFE_device_base.hpp"
#include "BFE_window_hs.hpp"

#include <string>
#include <vector>

namespace BFE {


	class BFEDeviceHS : public BFEDeviceBase {
	public:
#ifdef NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif
		BFEDeviceHS(BFEWindowHS& window);
		~BFEDeviceHS();

		BFEDeviceHS(const BFEDeviceHS&) = delete;
		void operator=(const BFEDeviceHS&) = delete;
		BFEDeviceHS(BFEDeviceHS&&) = delete;
		BFEDeviceHS& operator=(BFEDeviceHS&&) = delete;

	protected:
		/*void createLogicalDevice();
		void createInstance();
		void createSurface();
		bool isDeviceSuitable(VkPhysicalDevice device);
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
		std::vector<const char*> getRequiredExtensions();

		const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
		const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };*/
	};

}