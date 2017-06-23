cmake_minimum_required(VERSION 3.7)
project(memproject)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES meminfo.c meminfo.h mydef.h)
add_library(memproject STATIC ${SOURCE_FILES})

#include_directories(..)
#add_executable(zad test/zad.c)
