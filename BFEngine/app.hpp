#pragma once

#include "BFE_window_hs.hpp"
#include "BFE_device_hs.hpp"
#include "BFE_model.hpp"
#include "BFE_gameobject.hpp"
#include "BFE_renderer_hs.hpp"
#include "BFE_descriptors.hpp"
#include "BFE_texture.hpp"


#include <memory>
#include <vector>
#include <chrono>
namespace BFE {
	class App {
	public:
		App(boost::lockfree::queue<BFE::Frame*>& frameQueue);
		~App();
		void run();
	private:
		boost::lockfree::queue<BFE::Frame*>& frameQueue;
		
		void loadGameObjects();
		BFEWindowHS bfeWindow{ WIDTH, HEIGHT};
		BFEDeviceHS bfeDevice{ bfeWindow };

		size_t pid = bfeDevice.allocateCommandPool();
		BFERendererHS bfeRenderer{pid, bfeWindow, bfeDevice, frameQueue };

		std::unique_ptr<BFEDescriptorPool> globalPool{};
		std::vector<BFEGameObject> gameObjects;
		App(const App&);
		App& operator=(const App&);

	};
}