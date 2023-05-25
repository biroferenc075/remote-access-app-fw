#include "BFE_device_hs.hpp"

namespace BFE {
	BFEDeviceHS::BFEDeviceHS(BFEWindowHS& window) : BFEDeviceBase(window) {
		createInstance();
		setupDebugMessenger();
		pickPhysicalDevice();
		createLogicalDevice();
	}
	BFEDeviceHS::~BFEDeviceHS() {

	}
}