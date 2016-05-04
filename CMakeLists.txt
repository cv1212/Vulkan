cmake_minimum_required(VERSION 2.6)
project(VulkanBugs)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-sign-compare -std=c++11")

# The MAJOR number of the version we're building, used in naming
# vulkan-(major).dll (and other files).
set(MAJOR "1")

# Set includes
if(WIN32)
    include_directories("$ENV{VULKAN_SDK}/Include")
else()
    include_directories("/usr/include/vulkan")
endif()

# Find vulkan library
if(WIN32)
    set (VULKAN_LOADER_NAME "vulkan-${MAJOR}")
else()
    set (VULKAN_LOADER_NAME "vulkan")
endif()

find_library(VULKAN_LOADER NAMES ${VULKAN_LOADER_NAME})

# Create executables
add_executable(ResetFencesBug ResetFencesBug.cpp)
target_link_libraries(ResetFencesBug ${VULKAN_LOADER})

add_executable(CreateInstanceLimitBug CreateInstanceLimitBug.cpp)
target_link_libraries(CreateInstanceLimitBug ${VULKAN_LOADER})
