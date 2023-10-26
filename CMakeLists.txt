# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)
project ("remote-access-application-framework")

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(PROJECT_ROOT_DIR_SRC ${PROJECT_SOURCE_DIR})
set(PROJECT_ROOT_DIR_BIN ${PROJECT_BINARY_DIR})
set(CMAKE_DEBUG_FIND_MODE 1)

set(GLFW3_ROOT "C:/SDK/glfw-3.3.8.bin.WIN64") 
set(GLFW3_LIB "C:/SDK/glfw-3.3.8.bin.WIN64/lib-vc2022") 
set(glm_DIR "C:/SDK/glm-0.9.9.8/glm/cmake/glm")
set(glm_INCLUDE_DIR "C:/SDK/glm-0.9.9.8/glm")
set(Vulkan_LIBRARY "D:/SDK/VulkanSDK/1.3.236.0/Lib")
set(Vulkan_INCLUDE_DIR "D:/SDK/VulkanSDK/1.3.236.0/Include")
set(Boost_INCLUDE_DIR "D:/SDK/boost/boost_1_81_0")
set(Boost_LIBRARY "D:/SDK/boost/boost_1_81_0/stage/lib")
set(LIBS "${PROJECT_SOURCE_DIR}/libs")
set(LZ4_LIBRARY "${PROJECT_SOURCE_DIR}/libs/lz4")

 find_package(Vulkan REQUIRED)
# find_package(glm REQUIRED)
# find_package(GLFW3 3.3 REQUIRED)
# find_package(Boost REQUIRED)

list(APPEND INCLUDES
    ${glm_INCLUDE_DIR} ${Vulkan_INCLUDE_DIR} ${Boost_INCLUDE_DIR} ${LIBS})

list(APPEND LIBRARIES
    ${Vulkan_LIBRARIES}
    ${Boost_LIBRARY}
    ${GLFW3_LIB}
    ${LZ4_LIBRARY}
    )


include_directories(${INCLUDES})
link_directories(${LIBRARIES})
link_libraries(glfw3 glfw3_mt liblz4)


# Include sub-projects.
add_subdirectory ("src/BFE-Core")
add_subdirectory ("src/BFE-Headless")
add_subdirectory ("src/StreamingServer")
add_subdirectory ("src/StreamingClient")

