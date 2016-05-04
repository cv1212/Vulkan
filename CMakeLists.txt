cmake_minimum_required(VERSION 2.6)
project(VulkanBugs)

if(WIN32)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CRT_SECURE_NO_WARNINGS -D_USE_MATH_DEFINES")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_CRT_SECURE_NO_WARNINGS -D_USE_MATH_DEFINES")

    # If MSVC, disable some mismatch warnings.
    if (MSVC)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4267 /wd4244 /wd4804")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4267 /wd4244 /wd4804")
    endif()
else()
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-sign-compare -std=c++11")
endif()
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
add_executable(CreateDeviceLimit CreateDeviceLimit.cpp)
target_link_libraries(CreateDeviceLimit ${VULKAN_LOADER})
