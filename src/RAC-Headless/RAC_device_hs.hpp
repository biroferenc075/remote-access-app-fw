#pragma once

#include "../RAC-Core/RAC_device_base.hpp"
#include "RAC_window_hs.hpp"

#include <string>
#include <vector>

namespace RAC {


	class RACDeviceHS : public RACDeviceBase {
	public:
#ifdef NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif
		RACDeviceHS(RACWindowHS& window);
		~RACDeviceHS();

		RACDeviceHS(const RACDeviceHS&) = delete;
		void operator=(const RACDeviceHS&) = delete;
		RACDeviceHS(RACDeviceHS&&) = delete;
		RACDeviceHS& operator=(RACDeviceHS&&) = delete;

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