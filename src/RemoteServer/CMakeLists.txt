cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

list(APPEND SRC networkMod.cpp MyApplication.cpp KeyboardCameraController.cpp MouseCameraController.cpp main.cpp)
list(APPEND HDR networkMod.hpp MyApplication.hpp KeyboardCameraController.hpp compressionMod.hpp inputEvents.hpp)

add_executable (RemoteServer ${SRC} ${HDR})

target_link_libraries(RemoteServer RAC-Core)
target_link_libraries(RemoteServer RAC-Headless)

include_directories(${INCLUDES})
link_directories(${LIBRARIES})

