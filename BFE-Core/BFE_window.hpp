#pragma once
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
namespace BFE {
	class BFEWindow {
	public:
		BFEWindow(int _width, int _height, std::string _name);
		~BFEWindow();

		bool shouldClose() {
			return glfwWindowShouldClose(window);
		}
		VkExtent2D getExtent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }
		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
		bool wasWindowResized() { return framebufferResized; }
		void resetWindowResizedFlag() { framebufferResized = false; }
		GLFWwindow* getGLFWwindow() const { return window; }
	
	private:
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
		void initWindow();
		
		int width;
		int height;
		GLFWwindow* window;
		std::string name;
		bool framebufferResized = false;

		BFEWindow(const BFEWindow&);
		BFEWindow& operator=(const BFEWindow&);
	};
}