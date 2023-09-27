#pragma once
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "../BFE-Core/BFE_window_base.hpp"

namespace BFE {
	class BFEWindowHS : public BFEWindowBase {
	public:
		BFEWindowHS(int _width, int _height) : BFEWindowBase{ _width , _height } {}
		~BFEWindowHS() {}

		bool shouldClose() {
			return shouldClose_;
		}
	private:
		void initWindow();
		static void framebufferResizeCallback(BFEWindowHS* window, int width, int height);
		bool shouldClose_ = false;
		BFEWindowHS(const BFEWindowHS&);
		BFEWindowHS& operator=(const BFEWindowHS&);
	};
}