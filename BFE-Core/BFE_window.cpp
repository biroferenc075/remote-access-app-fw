#include "BFE_window.hpp"
#include <stdexcept>
namespace BFE {
	BFEWindow::BFEWindow(int _width, int _height, std::string _name) : width{ _width }, height{ _height }, name{_name} {
		initWindow();
	}
	BFEWindow::~BFEWindow() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void BFEWindow::initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}


	void BFEWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create windows surface!");
		}
	}


	void BFEWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto bfeWindow = reinterpret_cast<BFEWindow* >(glfwGetWindowUserPointer(window));
		bfeWindow->framebufferResized = true;
		bfeWindow->width = width;
		bfeWindow->height = height;
	
	}
}