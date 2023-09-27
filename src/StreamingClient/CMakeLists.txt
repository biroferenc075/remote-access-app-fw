# CMakeList.txt : CMake project for StreamingClient, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# Add source to this project's executable.
list(APPEND SRC decompressionMod.cpp displayMod.cpp networkMod.cpp StreamingClient.cpp main.cpp)
list(APPEND HDR decompressionMod.hpp displayMod.hpp networkMod.hpp streamingClient.hpp)

add_executable (StreamingClient ${SRC} ${HDR})
target_link_libraries(StreamingClient BFE-Core)
target_link_libraries(StreamingClient BFE-Headless)
# target_link_libraries(StreamingClient BFEngine)

include_directories(${INCLUDES})
link_directories(${LIBRARIES})

target_link_directories(StreamingClient PRIVATE ${Vulkan_LIBRARY} "C:/SDK/glfw-3.3.8.bin.WIN64/lib-vc2022" )
target_include_directories(StreamingClient PRIVATE ${GLFW3_INCLUDE_DIR} ${Vulkan_INCLUDE_DIR} ${glm_INCLUDE_DIR} ${LIBS})

