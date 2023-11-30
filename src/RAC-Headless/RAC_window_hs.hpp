#pragma once
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "../RAC-Core/RAC_window_base.hpp"

namespace RAC {
	class RACWindowHS : public RACWindowBase {
	public:
		RACWindowHS(int _width, int _height) : RACWindowBase{ _width , _height } {}
		~RACWindowHS() {}

		bool shouldClose() {
			return shouldClose_;
		}
	private:
		void initWindow();
		static void framebufferResizeCallback(RACWindowHS* window, int width, int height);
		bool shouldClose_ = false;
		RACWindowHS(const RACWindowHS&);
		RACWindowHS& operator=(const RACWindowHS&);
	};
}