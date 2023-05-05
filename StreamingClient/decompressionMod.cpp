#include "decompressionMod.hpp"

void sc::DecompressionModule::submitToQueue(Frame* frame)
{
	frameQueue.push(frame);
	sem.post();
	std::cout << "decomp submit\n";
}

void sc::DecompressionModule::run()
{	
	try {
		std::cout << "decomp init\n";
		isReady = true;

		boost::unique_lock<boost::mutex> lock(mut);
		while (!everyoneReady)
		{
			readyCond.wait(lock);
		}
		lock.unlock();
		std::cout << "decomp start\n";
		while (true) {
			std::cout << "decomp waiting\n";
			sem.wait();
			std::cout << "decomp working\n";
			Frame* fr;
			while (frameQueue.pop(fr)) {
				BFEImage::Builder* builder = new BFEImage::Builder();
				builder->loadImage(fr->size, fr->data, WIDTH, HEIGHT, 4);
				BFEImage* img = new BFEImage(pid, dm.bfeDevice, *builder, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
				dm.submitToQueue(img);
				//delete fr;
				delete builder;
				std::cout << "submitted frame to display, size: " << (int)fr->size << " data: " << reinterpret_cast<void*>(fr->data) << endl;
			}
		}
		std::cout << "decomp ended\n";
	}
	catch (std::exception e) {
		std::cout << e.what();
	}
}

sc::DecompressionModule::DecompressionModule(bool& readyFlag, boost::condition_variable& readyCond, boost::mutex& mut, DisplayModule& dm, BFEDevice& bfeDevice) : everyoneReady(readyFlag), readyCond(readyCond), mut(mut), dm(dm), sem(0) {
	std::cout << "decomp ctor\n";
	pid = bfeDevice.allocateCommandPool();
}
//TODO change semaphore value to queue size
