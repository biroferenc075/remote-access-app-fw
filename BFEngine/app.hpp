#pragma once

#include "BFE_window.hpp"
#include "BFE_device.hpp"
#include "BFE_model.hpp"
#include "BFE_gameobject.hpp"
#include "BFE_renderer.hpp"
#include "BFE_descriptors.hpp"
#include "BFE_texture.hpp"

#include <memory>
#include <vector>
#include <chrono>
namespace BFE {
	class App {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		App();
		~App();
		void run();
	private:
		void loadGameObjects();
		BFEWindow bfeWindow{ WIDTH, HEIGHT, "Vulkan" };
		BFEDevice bfeDevice{ bfeWindow };
		BFERenderer bfeRenderer{ bfeWindow, bfeDevice };

		std::unique_ptr<BFEDescriptorPool> globalPool{};
		std::vector<BFEGameObject> gameObjects;
		App(const App&);
		App& operator=(const App&);
	};
}