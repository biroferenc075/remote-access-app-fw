#pragma once
#include "consts.hpp"
#include "frame.hpp"

#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/lockfree/queue.hpp>
#include "lz4/lz4frame.h"

namespace RAC {
	class CompressionModule {
    public: 
        static void EncodeLZ4(Frame* f) {
            size_t compressedBufferSize = LZ4F_compressFrameBound(f->size, nullptr);
            unsigned char* compressedBuffer = (unsigned char*)malloc(compressedBufferSize);

            // Compress data
            int compressedSize = LZ4F_compressFrame(
                (char*)compressedBuffer,
                compressedBufferSize,
                (char*)f->data,
                f->size,
                nullptr
            );

            free(f->data);
            f->data = compressedBuffer;
            f->size = compressedBufferSize;

        }
	};
}