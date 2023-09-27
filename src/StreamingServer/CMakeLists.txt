# CMakeList.txt : CMake project for StreamingServer, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

list(APPEND SRC StreamingServer.cpp MyApplication.cpp KeyboardCameraController.cpp main.cpp)
list(APPEND HDR StreamingServer.hpp MyApplication.hpp KeyboardCameraController.hpp)

add_executable (StreamingServer ${SRC} ${HDR} )

target_link_libraries(StreamingServer BFE-Core)
target_link_libraries(StreamingServer BFE-Headless)

#target_link_directories(StreamingServer PUBLIC ${Vulkan_LIBRARY} Vulkan::Vulkan)

include_directories(${INCLUDES})
link_directories(${LIBRARIES})

