#pragma once
#include "consts.hpp"
#include "frame.hpp"
#include "../RAC-Core/RAC_image.hpp"
#include "displayMod.hpp"

#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/lockfree/queue.hpp>
using namespace std;
using namespace rc;
using boost::lockfree::queue;

namespace rc {
	class DecompressionModule {
	public:
		void submitToQueue(Frame* frame);
		void run();
		DecompressionModule(bool& readyFlag, boost::condition_variable& readyCond, boost::mutex& mut, DisplayModule& dm, RACDevice& racDevice);

		bool ready() {
			return isReady;
		}
		size_t pid;
	private:
		DisplayModule& dm;
		queue<Frame*> frameQueue{size_t(16)};
		
		bool& everyoneReady;
		bool isReady = false;
		boost::condition_variable& readyCond;
		boost::mutex& mut;

		boost::interprocess::interprocess_semaphore sem;
	};
}