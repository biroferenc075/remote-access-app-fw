#pragma once

#include "BFE_window.hpp"
#include "BFE_device.hpp"
#include "BFE_swap_chain.hpp"
#include "BFE_renderer.hpp"
#include "BFE_image.hpp"

#include "frame.hpp"
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/lockfree/queue.hpp>
#include <chrono>

using boost::lockfree::queue;
using namespace BFE;

namespace sc {
	class DisplayModule {
	public:
		static constexpr int WIDTH = 600;
		static constexpr int HEIGHT = 400;
		static constexpr int FRAMERATE = 30;

		DisplayModule(boost::asio::io_context& io_context, bool& readyFlag, boost::condition_variable& readyCond, boost::mutex& mut);
		~DisplayModule();
		void run();
		void handler(const boost::system::error_code& error, BFE::BFERenderer* bfeRenderer);
		void submitToQueue(BFEImage* img);
		
		BFEWindow bfeWindow{ WIDTH, HEIGHT, "Vulkan" }; // TODO pull out window and device intialization
		BFEDevice bfeDevice{ bfeWindow };
		BFERenderer bfeRenderer{ bfeWindow, bfeDevice };
		bool ready() {
			return isReady;
		}
	private:
		boost::asio::io_context& io_context_;
		boost::asio::steady_timer timer;
		std::chrono::steady_clock::duration dur = std::chrono::steady_clock::duration(1'000'000'000 / FRAMERATE);
		
		bool& everyoneReady;
		bool isReady = false;
		boost::condition_variable& readyCond;
		boost::mutex& mut;
		queue<BFEImage*> imageQueue{ size_t(16) };

		bool ioerror = false;
	};
}
