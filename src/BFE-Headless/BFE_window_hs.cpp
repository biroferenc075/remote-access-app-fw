#include "BFE_window_hs.hpp"
#include <stdexcept>
namespace BFE {
	void BFEWindowHS::framebufferResizeCallback(BFEWindowHS* window, int width, int height) {
		window->width = width;
		window->height = height;
		window->framebufferResized = true;
	}

	void BFEWindowHS::initWindow() {
		
	}
}