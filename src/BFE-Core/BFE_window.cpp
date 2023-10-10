#include "BFE_window.hpp"
#include <stdexcept>
namespace BFE {
	BFEWindow::BFEWindow(int _width, int _height, std::string _name) : BFEWindowBase{ _width, _height }, name{_name} {
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
		
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);
		glfwSetKeyCallback(window, KeyCallback);
		glfwSetMouseButtonCallback(window, MouseButtonCallback);
	}


	void BFEWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create windows surface!");
		}
	}

	void BFEWindow::registerCallbacks(boost::function<void(int, int)> windowResizedCb, boost::function<void(int, int, int, int)> keyboardCb, boost::function<void(double, double, int, int, int)> mouseButtonCb)
	{
		this->windowResizedCb = windowResizedCb;
		this->keyboardCb = keyboardCb;
		this->mouseButtonCb = mouseButtonCb;
	}


	void BFEWindow::FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto bfeWindow = reinterpret_cast<BFEWindow* >(glfwGetWindowUserPointer(window));
		bfeWindow->framebufferResized = true;
		bfeWindow->width = width;
		bfeWindow->height = height;

		if (bfeWindow->windowResizedCb) {
			bfeWindow->windowResizedCb(width, height);
		}
	}
	void BFEWindow::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		auto bfeWindow = reinterpret_cast<BFEWindow*>(glfwGetWindowUserPointer(window));

		if (bfeWindow->keyboardCb) {
			bfeWindow->keyboardCb(key, scancode, action , mods);
		}
	}
	void BFEWindow::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
		auto bfeWindow = reinterpret_cast<BFEWindow*>(glfwGetWindowUserPointer(window));
		double x, y;
		glfwGetCursorPos(window, &x, &y);

		if (bfeWindow->mouseButtonCb) {
			bfeWindow->mouseButtonCb(x, y, button, action, mods);
		}
	}
}