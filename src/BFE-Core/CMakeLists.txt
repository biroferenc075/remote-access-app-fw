# CMakeList.txt : CMake project for BFE-Core, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add source to this project's executable.
list(APPEND SRC BFE_buffer.cpp BFE_camera.cpp BFE_descriptors.cpp BFE_device.cpp BFE_device_base.cpp BFE_image.cpp BFE_model.cpp BFE_pipeline.cpp BFE_renderer.cpp BFE_renderer_base.cpp BFE_render_system.cpp BFE_swap_chain.cpp BFE_texture.cpp BFE_window.cpp)
list(APPEND HDR BFE_buffer.hpp BFE_camera.hpp BFE_descriptors.hpp BFE_device.hpp BFE_device_base.hpp BFE_image.hpp BFE_model.hpp BFE_pipeline.hpp BFE_renderer.hpp BFE_renderer_base.hpp BFE_render_system.hpp BFE_swap_chain.hpp BFE_texture.hpp BFE_window.hpp BFE_frame_info.hpp BFE_gameobject.hpp BFE_utils.h BFE_window_base.hpp)

add_library (BFE-Core STATIC ${SRC} ${HDR})
target_link_directories(BFE-Core PUBLIC ${Vulkan_LIBRARIES} ${glm_INCLUDE_DIR})
target_link_libraries(BFE-Core INTERFACE vulkan-1 ${GLFW3_LIBRARY})
target_include_directories(BFE-Core PRIVATE ${GLFW3_INCLUDE_DIR} ${Vulkan_INCLUDE_DIR} ${glm_INCLUDE_DIR} ${LIBS})
include_directories(${INCLUDES})
link_directories(${LIBRARIES})


# TODO: Add tests and install targets if needed.
