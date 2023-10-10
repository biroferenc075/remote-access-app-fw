#pragma once
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <boost/function.hpp>

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
		void registerCallbacks(boost::function<void(int, int)> windowResizedCb, boost::function<void(int, int, int, int)> keyboardCb, boost::function<void(double, double, int, int, int)> mouseButtonCb);
	
	private:
		static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		void initWindow();
		GLFWwindow* window;
		std::string name;
		BFEWindow(const BFEWindow&);
		BFEWindow& operator=(const BFEWindow&);

		boost::function<void(int, int)> windowResizedCb;
		boost::function<void(int, int, int, int)> keyboardCb;
		boost::function<void(double, double, int, int, int)> mouseButtonCb;
	};
}