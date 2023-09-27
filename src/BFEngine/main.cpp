#include "app.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "../StreamingServer/StreamingServer.hpp"
int main() {
boost::lockfree::queue<BFE::Frame*> frameQueue{ size_t(16) };
BFE::App app(frameQueue);
	try {
		boost::thread t(boost::bind(&runServer, &frameQueue));
		t.detach();
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}