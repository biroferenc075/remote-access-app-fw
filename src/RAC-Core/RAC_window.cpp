#include "RAC_window.hpp"
#include <stdexcept>
namespace RAC {
	RACWindow::RACWindow(int _width, int _height, std::string _name) : RACWindowBase{ _width, _height }, name{_name} {
		initWindow();
	}
	RACWindow::~RACWindow() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void RACWindow::initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
		
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);
		glfwSetKeyCallback(window, KeyCallback);
		glfwSetMouseButtonCallback(window, MouseButtonCallback);
	}


	void RACWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create windows surface!");
		}
	}

	void RACWindow::registerCallbacks(boost::function<void(int, int)> windowResizedCb, boost::function<void(int, int, int, int)> keyboardCb, boost::function<void(double, double, int, int, int)> mouseButtonCb)
	{
		this->windowResizedCb = windowResizedCb;
		this->keyboardCb = keyboardCb;
		this->mouseButtonCb = mouseButtonCb;
	}


	void RACWindow::FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto racWindow = reinterpret_cast<RACWindow* >(glfwGetWindowUserPointer(window));
		racWindow->framebufferResized = true;
		racWindow->width = width;
		racWindow->height = height;

		if (racWindow->windowResizedCb) {
			racWindow->windowResizedCb(width, height);
		}
	}
	void RACWindow::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		auto racWindow = reinterpret_cast<RACWindow*>(glfwGetWindowUserPointer(window));

		if (racWindow->keyboardCb) {
			racWindow->keyboardCb(key, scancode, action , mods);
		}
	}
	void RACWindow::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
		auto racWindow = reinterpret_cast<RACWindow*>(glfwGetWindowUserPointer(window));
		double x, y;
		glfwGetCursorPos(window, &x, &y);

		if (racWindow->mouseButtonCb) {
			racWindow->mouseButtonCb(x, y, button, action, mods);
		}
	}
}