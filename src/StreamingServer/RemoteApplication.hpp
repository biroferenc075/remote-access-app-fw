#pragma once


#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/array.hpp>

#include "../BFE-Headless/BFE_window_hs.hpp"
#include "../BFE-Headless/BFE_device_hs.hpp"
#include "../BFE-Headless/BFE_renderer_hs.hpp"
#include "../BFE-Core/BFE_model.hpp"
#include "../BFE-Core/BFE_gameobject.hpp"
#include "../BFE-Core/BFE_descriptors.hpp"
#include "../BFE-Core/BFE_texture.hpp"
#include "../BFE-Core/BFE_camera.hpp"
#include "../BFE-Core/BFE_render_system.hpp"

#include "networkMod.hpp"
#include "inputEvents.hpp"
#include "consts.hpp"

#include <ctime>
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <chrono>
#include <set>

using namespace BFE;

class RemoteApplication {
protected:
	boost::lockfree::queue<Frame*> frameQueue{size_t(16)};
	boost::lockfree::queue<InputEvent*> inputEventQueue{ size_t(64) };
	BFEWindowHS bfeWindow{ WIDTH, HEIGHT };
	BFEDeviceHS bfeDevice{ bfeWindow };

	size_t pid = bfeDevice.allocateCommandPool();
	BFERendererHS bfeRenderer{ pid, bfeWindow, bfeDevice, frameQueue };
private:
	// modules go here
	void registerCallbacks() {
		
	}
	void init() {
		registerCallbacks();
	}
protected:
	bool connected = false;
	bool started = false;
	//keys pressed list
	//mouse pos
	//resolution
	virtual void onStart() {}
	virtual void onConn() {}
	virtual void onDisconn() {}
	virtual void onUpdate(float totalTime, float deltaTime, std::set<int>& keysPressed, std::set<int>& keysReleased, std::set<int>& keysHeld) {

	}
	virtual void onDraw(float totalTime, float deltaTime) {

	}
	virtual void onKeyPress(int key, int mods, int scancode) {

	}
	virtual void onKeyRelease(int key, int mods, int scancode) {

	}
	virtual void onMouseMove(double dX, double dY, double x, double y) {

	}
	virtual void onMousePress(double x, double y, int button, int mods) {

	}
	virtual void onMouseRelease(double x, double y, int button, int mods) {

	}
	virtual void onWindowEvent(int width, int height) {

	}
	void HandleWindowResizeEvent(BFE::InputEvent* ev)
	{
		WindowResizeEvent* event = static_cast<WindowResizeEvent*>(ev);
		this->onWindowEvent(event->width, event->height);
	}
	void HandleKeyboardEvent(BFE::InputEvent* ev, std::set<int>& keysPressed, std::set<int>& keysReleased)
	{
		KeyboardEvent* event = static_cast<KeyboardEvent*>(ev);
		switch (event->action) {
		case GLFW_PRESS:
			keysPressed.insert(event->key);
			onKeyPress(event->key, event->mods, event->scancode);
			break;
		case GLFW_RELEASE:
			keysReleased.insert(event->key);
			onKeyRelease(event->key, event->mods, event->scancode);
			break;
		}
	}
	void HandleMousePollEvent(BFE::InputEvent* ev)
	{
		MousePollEvent* event = static_cast<MousePollEvent*>(ev);
		static double prevX = 0.0, prevY = 0.0;

		this->onMouseMove(prevX - event->xpos, prevY - event->ypos, event->xpos, event->ypos);
		prevX = event->xpos;
		prevY = event->ypos;
	}
	void HandleMousePressEvent(BFE::InputEvent* ev, std::set<int>& keysPressed, std::set<int>& keysReleased)
	{
		MousePressEvent* event = static_cast<MousePressEvent*>(ev);
		switch (event->action) {
		case GLFW_PRESS:
			this->onMousePress(event->xpos, event->ypos, event->button, event->mods);
			keysPressed.insert(event->button);
			break;
		case GLFW_RELEASE:
			this->onMouseRelease(event->xpos, event->ypos, event->button, event->mods);
			keysReleased.insert(event->button);
			break;
		}
	}
public:
	RemoteApplication() {
		init();
	}
	int run() {
		bool shouldStop = false;

		boost::asio::io_context io_context;
		tcp_server server(io_context, frameQueue, inputEventQueue, shouldStop, boost::bind(&RemoteApplication::onConn, this), boost::bind(&RemoteApplication::onDisconn, this));
		boost::thread thr(boost::bind(&boost::asio::io_service::run, &io_context));
		thr.detach();

		onStart();

		float t = 0;
		auto currentTime = std::chrono::high_resolution_clock::now();
		std::set<int> keysPressed;
		std::set<int> keysReleased;
		std::set<int> keysHeld;
		while (!bfeWindow.shouldClose()) {

			InputEvent* ev;
			while (inputEventQueue.pop(ev)) {
				switch (ev->getTypeId()) {
				case 0:
					HandleMousePressEvent(ev, keysPressed, keysReleased);
					break;
				case 1:
					HandleMousePollEvent(ev);
					break;
				case 2:
					HandleKeyboardEvent(ev, keysPressed, keysReleased);
					break;
				case 3:
					HandleWindowResizeEvent(ev);
					break;
				}
				delete ev;
			}

			for (auto it = keysPressed.begin(); it != keysPressed.end(); it++) {
				keysHeld.insert(*it);
			}
			for (auto it = keysReleased.begin(); it != keysReleased.end(); it++) {
				keysHeld.erase(*it);
			}

			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			t += frameTime;
			currentTime = newTime;

			onUpdate(t, frameTime, keysPressed, keysReleased, keysHeld);
			if(connected)
				onDraw(t, frameTime);
			
			keysPressed.clear();
			keysReleased.clear();
			Sleep(1000.0f / FRAMERATE - frameTime);
		}


		vkDeviceWaitIdle(bfeDevice.device());
		return 0;
	}
	
};