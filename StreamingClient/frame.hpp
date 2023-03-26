#pragma once
#include <boost/asio.hpp>

namespace sc {
    struct Frame {
        uint32_t id;
        char* data;
        size_t size;

        Frame(size_t size) {
            static uint32_t gid = 0;
            id = gid++;
            data = (char*)malloc(size); //TODO allocator
            this->size = size;
        }
        ~Frame() {
            free(data);
        }
    };
}