# CMakeList.txt : CMake project for BFE-Headless, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# Add source to this project's executable.
list(APPEND SRC BFE_device_hs.cpp BFE_renderer_hs.cpp BFE_window_hs.cpp)
list(APPEND HDR BFE_device_hs.hpp BFE_renderer_hs.hpp BFE_window_hs.hpp)

add_library (BFE-Headless STATIC ${SRC} ${HDR})
target_link_libraries(BFE-Headless BFE-Core)
target_include_directories(BFE-Headless PUBLIC ${GLFW3_INCLUDE_DIR} ${Vulkan_INCLUDE_DIR} ${glm_INCLUDE_DIR} ${LIBS})
include_directories(${INCLUDES})
link_directories(${LIBRARIES})

# TODO: Add tests and install targets if needed.
