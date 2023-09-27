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

#include "StreamingServer.hpp"

#include "consts.hpp"

#include <ctime>
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <chrono>

using namespace BFE;

class RemoteApplication {
protected:
	boost::lockfree::queue<Frame*> frameQueue{size_t(16)};
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
	//keys pressed list
	//mouse pos
	//resolution
	virtual void onStart() {

	}
	virtual void onConn() {
	}
	virtual void onDisconn() {

	}
	virtual void onUpdate(float totalTime, float deltaTime) {

	}
	virtual void onDraw(float totalTime, float deltaTime) {

	}
	virtual void onKey() {

	}
	virtual void onMouse() {

	}
	virtual void onWindowEvent() {

	}
public:
	RemoteApplication() {
		init();
	}
	int run() {
		bool shouldStop = false;

		boost::asio::io_context io_context;
		tcp_server server(io_context, frameQueue, shouldStop);
		boost::thread thr(boost::bind(&boost::asio::io_service::run, &io_context));
		thr.detach();

		server.registerConnectCallback(boost::bind(&RemoteApplication::onConn, this));

		onStart();

		float t = 0;
		auto currentTime = std::chrono::high_resolution_clock::now();
		while (!bfeWindow.shouldClose()) {
			glfwPollEvents();

			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			t += frameTime;
			currentTime = newTime;

			onUpdate(t, frameTime);
			if(connected)
				onDraw(t, frameTime);

			Sleep(1000.0f / FRAMERATE - frameTime);
		}


		vkDeviceWaitIdle(bfeDevice.device());
		return 0;
	}
};