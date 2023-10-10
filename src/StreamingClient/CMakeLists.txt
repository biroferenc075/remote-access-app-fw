# CMakeList.txt : CMake project for StreamingClient, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# Add source to this project's executable.
list(APPEND SRC decompressionMod.cpp displayMod.cpp networkMod.cpp StreamingClient.cpp main.cpp)
list(APPEND HDR decompressionMod.hpp displayMod.hpp networkMod.hpp streamingClient.hpp inputEvents.hpp)

add_executable (StreamingClient ${SRC} ${HDR})
target_link_libraries(StreamingClient BFE-Core)
target_link_libraries(StreamingClient BFE-Headless)



