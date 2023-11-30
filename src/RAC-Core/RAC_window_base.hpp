#pragma once
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
namespace RAC {
	class RACWindowBase {
	public:
		RACWindowBase(int _width, int _height) : width(_width), height(_height) {}

		virtual bool shouldClose() = 0;
		VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }
		bool wasWindowResized() { return framebufferResized; }
		void resetWindowResizedFlag() { framebufferResized = false; }
	protected:
		virtual void initWindow() = 0;
		int width;
		int height;
		bool framebufferResized = false;

		RACWindowBase(const RACWindowBase&);
		RACWindowBase& operator=(const RACWindowBase&);
	};
}