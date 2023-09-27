#pragma once

#include "consts.hpp"

#include "../BFE-Core/BFE_window.hpp"
#include "../BFE-Core/BFE_device.hpp"
#include "../BFE-Core/BFE_swap_chain.hpp"
#include "../BFE-Core/BFE_renderer.hpp"
#include "../BFE-Core/BFE_image.hpp"


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
		size_t pid;
		DisplayModule(boost::asio::io_context& io_context, bool& readyFlag, boost::condition_variable& readyCond, boost::mutex& mut, BFEWindow& bfeWindow, BFEDevice& bfeDevice);
		~DisplayModule();
		void run();
		void handler(const boost::system::error_code& error, BFE::BFERenderer* bfeRenderer);
		void submitToQueue(BFEImage* img);
		
		BFEWindow& bfeWindow;
		BFEDevice& bfeDevice;
		BFERenderer bfeRenderer;
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
