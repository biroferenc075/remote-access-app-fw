#pragma once
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "BFE_window_base.hpp"
namespace BFE {
	class BFEWindow : public BFEWindowBase {
	public:
		BFEWindow(int _width, int _height, std::string _name);
		~BFEWindow();

		bool shouldClose() {
			return glfwWindowShouldClose(window);
		}
		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
		GLFWwindow* getGLFWwindow() const { return window; }
	
	private:
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
		void initWindow();
		GLFWwindow* window;
		std::string name;
		BFEWindow(const BFEWindow&);
		BFEWindow& operator=(const BFEWindow&);
	};
}