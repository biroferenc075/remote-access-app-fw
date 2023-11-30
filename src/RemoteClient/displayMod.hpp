#pragma once

#include "consts.hpp"

#include "../RAC-Core/RAC_window.hpp"
#include "../RAC-Core/RAC_device.hpp"
#include "../RAC-Core/RAC_swap_chain.hpp"
#include "../RAC-Core/RAC_renderer.hpp"
#include "../RAC-Core/RAC_image.hpp"


#include "frame.hpp"
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/lockfree/queue.hpp>
#include <chrono>

using boost::lockfree::queue;
using namespace RAC;

namespace rc {
	class DisplayModule {
	public:
		size_t pid;
		DisplayModule(boost::asio::io_context& io_context, bool& readyFlag, boost::condition_variable& readyCond, boost::mutex& mut, RACWindow& racWindow, RACDevice& racDevice);
		~DisplayModule();
		void run();
		void handler(const boost::system::error_code& error, RAC::RACRenderer* racRenderer);
		void submitToQueue(RACImage* img);
		
		RACWindow& racWindow;
		RACDevice& racDevice;
		RACRenderer racRenderer;
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
		queue<RACImage*> imageQueue{ size_t(16) };

		bool ioerror = false;
	};
}
