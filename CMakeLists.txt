cmake_minimum_required(VERSION 3.22)

project(dat-vfs)

add_library(dat-vfs STATIC)

target_include_directories(dat-vfs PUBLIC ./include)

target_sources(dat-vfs PRIVATE
        source/dat-vfs.cpp
        source/dat-path.cpp
)

# Examples
add_subdirectory(examples)

# Tests
set("DATVFS_ENABLE_TESTS" OFF CACHE BOOL "Whether to enable testing")
if (${DATVFS_ENABLE_TESTS})
    message("Testing has been enabled")
    add_subdirectory(tests)
else()
    message("Testing has been disabled")
endif ()