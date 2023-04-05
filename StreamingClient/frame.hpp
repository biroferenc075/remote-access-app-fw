#pragma once
#include <boost/asio.hpp>

namespace sc {
    struct Frame {
        size_t size;
        unsigned char* data;

        Frame(size_t size) {
            data = (unsigned char*)malloc(size);
            this->size = size;
        }
    };
}