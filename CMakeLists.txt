cmake_minimum_required(VERSION 3.22)

project(dat-vfs)

add_library(dat-vfs STATIC)

target_include_directories(dat-vfs PUBLIC ./include)

target_sources(dat-vfs PRIVATE
        source/dat-vfs.cpp
)

add_subdirectory(examples)