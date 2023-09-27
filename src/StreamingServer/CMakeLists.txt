# CMakeList.txt : CMake project for StreamingServer, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add source to this project's executable.

list(APPEND SRC StreamingServer.cpp MyApplication.cpp KeyboardCameraController.cpp main.cpp)
list(APPEND HDR StreamingServer.hpp MyApplication.hpp KeyboardCameraController.hpp)

add_executable (StreamingServer ${SRC} ${HDR} )
target_link_libraries(StreamingServer BFE-Core)
target_link_libraries(StreamingServer BFE-Headless)
# target_link_libraries(StreamingServer BFEngine)
target_link_directories(StreamingServer PUBLIC ${Vulkan_LIBRARY} Vulkan::Vulkan)
target_include_directories(StreamingServer PUBLIC ${GLFW3_INCLUDE_DIR} ${Vulkan_INCLUDE_DIR} ${glm_INCLUDE_DIR} ${LIBS})
include_directories(${INCLUDES})
link_directories(${LIBRARIES})
# TODO: Add tests and install targets if needed.
