#include "decompressionMod.hpp"
#include "lz4/lz4frame.h"

static void DecodeLZ4(Frame* f) {
	LZ4F_dctx* decompContext;
	LZ4F_createDecompressionContext(&decompContext, LZ4F_VERSION);

	size_t newsize = WIDTH * HEIGHT * 4;
	unsigned char* newdata = (unsigned char*)malloc(newsize);

	size_t srcSize = f->size;
	size_t dstSize = newsize;
	unsigned char* srcData = f->data;
	unsigned char* dstData = newdata;

	while (LZ4F_decompress(decompContext, dstData, &dstSize, srcData, &srcSize, nullptr) != 0) {
		srcData += srcSize;
		dstData += dstSize;

		srcSize = f->size - srcSize;
		dstSize = newsize - dstSize;
	}

	free(f->data);
	f->data = newdata;
	f->size = newsize;
}

void rc::DecompressionModule::submitToQueue(Frame* frame)
{
	frameQueue.push(frame);
	sem.post();
}

void rc::DecompressionModule::run()
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
				DecodeLZ4(fr);
				RACImage::Builder* builder = new RACImage::Builder();
				builder->loadImage(fr->size, fr->data, WIDTH, HEIGHT, 4);
				RACImage* img = new RACImage(pid, dm.racDevice, *builder, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
				dm.submitToQueue(img);
				delete builder;
			}
		}

	}
	catch (std::exception e) {
		std::cout << e.what();
	}
}

rc::DecompressionModule::DecompressionModule(bool& readyFlag, boost::condition_variable& readyCond, boost::mutex& mut, DisplayModule& dm, RACDevice& racDevice) : everyoneReady(readyFlag), readyCond(readyCond), mut(mut), dm(dm), sem(0) {
	pid = racDevice.allocateCommandPool();
}
