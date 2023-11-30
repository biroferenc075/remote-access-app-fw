cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

list(APPEND SRC RAC_device_hs.cpp RAC_renderer_hs.cpp RAC_window_hs.cpp)
list(APPEND HDR RAC_device_hs.hpp RAC_renderer_hs.hpp RAC_window_hs.hpp)

add_library (RAC-Headless STATIC ${SRC} ${HDR})
target_link_libraries(RAC-Headless RAC-Core)
target_include_directories(RAC-Headless PUBLIC ${GLFW3_INCLUDE_DIR} ${Vulkan_INCLUDE_DIR} ${glm_INCLUDE_DIR} ${LIBS})
include_directories(${INCLUDES})
link_directories(${LIBRARIES})

