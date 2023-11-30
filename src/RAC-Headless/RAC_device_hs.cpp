#include "RAC_device_hs.hpp"
#include "../RAC-Core/RAC_device_base.hpp"

namespace RAC {
	RACDeviceHS::RACDeviceHS(RACWindowHS& window) : RACDeviceBase(window) {
		createInstance();
		setupDebugMessenger();
		pickPhysicalDevice();
		createLogicalDevice();
	}
	RACDeviceHS::~RACDeviceHS() {

	}
}