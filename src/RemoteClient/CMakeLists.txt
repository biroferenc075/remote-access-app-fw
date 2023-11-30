cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

list(APPEND SRC decompressionMod.cpp displayMod.cpp networkMod.cpp RemoteClient.cpp main.cpp)
list(APPEND HDR decompressionMod.hpp displayMod.hpp networkMod.hpp RemoteClient.hpp inputEvents.hpp)

add_executable (RemoteClient ${SRC} ${HDR})
target_link_libraries(RemoteClient RAC-Core)
target_link_libraries(RemoteClient RAC-Headless)



