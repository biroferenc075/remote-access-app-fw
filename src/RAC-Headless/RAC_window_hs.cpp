#include "RAC_window_hs.hpp"
#include <stdexcept>
namespace RAC {
	void RACWindowHS::framebufferResizeCallback(RACWindowHS* window, int width, int height) {
		window->width = width;
		window->height = height;
		window->framebufferResized = true;
	}

	void RACWindowHS::initWindow() {
		
	}
}