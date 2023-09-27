# CMakeList.txt : CMake project for BFEngine, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# Add source to this project's executable.
list(APPEND SRC app.cpp keyboard_test.cpp main.cpp)
list(APPEND HDR app.hpp keyboard_test.hpp)

add_executable (BFEngine ${SRC} ${HDR})
target_link_libraries(BFEngine BFE-Core)
target_link_libraries(BFEngine BFE-Headless)

target_link_directories(BFEngine PUBLIC ${Vulkan_LIBRARY} Vulkan::Vulkan)
target_include_directories(BFEngine PUBLIC ${GLFW3_INCLUDE_DIR} ${Vulkan_INCLUDE_DIR} ${glm_INCLUDE_DIR} ${LIBS})
