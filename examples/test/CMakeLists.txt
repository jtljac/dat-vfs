cmake_minimum_required(VERSION 3.22)

project(dat-vfs-test)

add_executable(dat-vfs-test main.cpp)

target_link_libraries(dat-vfs-test dat-vfs)