#include "BFE_device_hs.hpp"
#include "../BFE-Core/BFE_device_base.hpp"

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