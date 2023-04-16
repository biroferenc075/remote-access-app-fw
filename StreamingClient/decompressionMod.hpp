#pragma once
#include "frame.hpp"
#include "BFE_image.hpp"
#include "displayMod.hpp"

#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/lockfree/queue.hpp>
using namespace std;
using namespace sc;
using boost::lockfree::queue;

namespace sc {
	class DecompressionModule {
	public:
		void submitToQueue(Frame* frame);
		void run();
		DecompressionModule(bool& readyFlag, boost::condition_variable& readyCond, boost::mutex& mut, DisplayModule& dm);

		bool ready() {
			return isReady;
		}
	private:
		DisplayModule& dm;
		queue<Frame*> frameQueue;
		
		bool& everyoneReady;
		bool isReady = false;
		boost::condition_variable& readyCond;
		boost::mutex& mut;
		queue<BFEImage* > imageQueue;

		boost::interprocess::interprocess_semaphore sem;
	};
}