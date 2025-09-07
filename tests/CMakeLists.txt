cmake_minimum_required(VERSION 3.22)

enable_testing()

CPMAddPackage("gh:catchorg/Catch2@3.10.0")
list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
include(Catch)

add_executable(dat-vfs-tests
        UnitTest.cpp
        TestDatPath.cpp
        TestDatVfsFile.cpp
        TestDatVfs.cpp
)

target_link_libraries(dat-vfs-tests PRIVATE Catch2::Catch2WithMain)
target_link_libraries(dat-vfs-tests PRIVATE dat-vfs)

# Start Testing
catch_discover_tests(dat-vfs-tests WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})