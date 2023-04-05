#include "decompressionMod.hpp"

void sc::DecompressionModule::submitToQueue(Frame* frame)
{
	frameQueue.push(frame);
	sem.post();
	std::cout << "decomp submit\n";
}

void sc::DecompressionModule::run()
{
	boost::lock_guard<boost::mutex> lock(mut);
	std::cout << "decomp run\n";
	while (true) {
		sem.wait();
		Frame* fr;
		while (frameQueue.pop(fr)) {
			BFEImage::Builder builder;
			builder.loadImage(fr->size, fr->data);
			BFEImage* img = new BFEImage(dm.bfeDevice, builder); 
			dm.submitToQueue(img);
			delete fr;
		}
	}
}

sc::DecompressionModule::DecompressionModule(bool& readyFlag, boost::condition_variable& readyCond, boost::mutex& mut, DisplayModule& dm) : readyFlag(readyFlag), readyCond(readyCond), mut(mut), dm(dm), sem(0) {} 
//TODO change semaphore value to queue size
