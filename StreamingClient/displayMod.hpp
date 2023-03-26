#pragma once

#include "BFE_window.hpp"
#include "BFE_device.hpp"
#include "BFE_swap_chain.hpp"
#include "BFE_renderer.hpp"

namespace sc {
	class DisplayModule {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		DisplayModule();
		~DisplayModule();
		void run();
	//private:
		//BFE::BFEWindow bfeWindow{ WIDTH, HEIGHT, "Vulkan" };
		//BFE::BFEDevice bfeDevice{ bfeWindow };
		//BFE::BFERenderer bfeRenderer{ bfeWindow, bfeDevice };
	};
}
