cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

list(APPEND SRC RAC_buffer.cpp RAC_camera.cpp RAC_descriptors.cpp RAC_device.cpp RAC_device_base.cpp RAC_image.cpp RAC_model.cpp RAC_pipeline.cpp RAC_renderer.cpp RAC_renderer_base.cpp RAC_render_system.cpp RAC_swap_chain.cpp RAC_texture.cpp RAC_window.cpp)
list(APPEND HDR RAC_buffer.hpp RAC_camera.hpp RAC_descriptors.hpp RAC_device.hpp RAC_device_base.hpp RAC_image.hpp RAC_model.hpp RAC_pipeline.hpp RAC_renderer.hpp RAC_renderer_base.hpp RAC_render_system.hpp RAC_swap_chain.hpp RAC_texture.hpp RAC_window.hpp RAC_frame_info.hpp RAC_gameobject.hpp RAC_utils.h RAC_window_base.hpp)

add_library (RAC-Core STATIC ${SRC} ${HDR})
target_link_directories(RAC-Core PUBLIC ${Vulkan_LIBRARIES} ${glm_INCLUDE_DIR})
target_link_libraries(RAC-Core INTERFACE vulkan-1 ${GLFW3_LIBRARY})
target_include_directories(RAC-Core PRIVATE ${GLFW3_INCLUDE_DIR} ${Vulkan_INCLUDE_DIR} ${glm_INCLUDE_DIR} ${LIBS})
include_directories(${INCLUDES})
link_directories(${LIBRARIES})

