cmake_minimum_required(VERSION 3.22)

enable_testing()

find_package(Catch2 REQUIRED)
include(CTest)
include(Catch)

add_executable(dat-vfs-tests
        unit-test.cpp
        dat-path-tests.cpp
        dat-vfs-file-test.cpp
        dat-vfs-tests.cpp

)

target_link_libraries(dat-vfs-tests PRIVATE Catch2::Catch2)
target_link_libraries(dat-vfs-tests PRIVATE dat-vfs)

# Start Testing
catch_discover_tests(dat-vfs-tests WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})