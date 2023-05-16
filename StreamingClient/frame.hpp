#pragma once
#include <boost/asio.hpp>
#include "consts.hpp"
namespace sc {
    struct Frame {
        size_t size;
        unsigned char* data;

        Frame(size_t size) {
            data = (unsigned char*)malloc(size);
            this->size = size;
        }

        Frame(size_t size, unsigned char* data) : size(size), data(data) {}
    };
}