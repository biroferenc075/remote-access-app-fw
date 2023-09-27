#include "decompressionMod.hpp"

void sc::DecompressionModule::submitToQueue(Frame* frame)
{
	frameQueue.push(frame);
	sem.post();
}

void sc::DecompressionModule::run()
{	
	try {
		isReady = true;

		boost::unique_lock<boost::mutex> lock(mut);
		while (!everyoneReady)
		{
			readyCond.wait(lock);
		}
		lock.unlock();
		while (true) {
			sem.wait();

			Frame* fr;
			while (frameQueue.pop(fr)) {
				BFEImage::Builder* builder = new BFEImage::Builder();
				builder->loadImage(fr->size, fr->data, WIDTH, HEIGHT, 4);
				BFEImage* img = new BFEImage(pid, dm.bfeDevice, *builder, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
				dm.submitToQueue(img);
				delete builder;
			}
		}

	}
	catch (std::exception e) {
		std::cout << e.what();
	}
}

sc::DecompressionModule::DecompressionModule(bool& readyFlag, boost::condition_variable& readyCond, boost::mutex& mut, DisplayModule& dm, BFEDevice& bfeDevice) : everyoneReady(readyFlag), readyCond(readyCond), mut(mut), dm(dm), sem(0) {
	pid = bfeDevice.allocateCommandPool();
}
