cmake_minimum_required(VERSION 3.0.2)
project(Zad1)
set(CMAKE_BUILD_TYPE Release)

add_definitions(-Wall)

include_directories(.)
file(GLOB SOURCES "./*.c")

add_library(ContactBook SHARED ${SOURCES})

add_library(ContactBookStatic STATIC ${SOURCES})

install(TARGETS ContactBook DESTINATION /usr/lib)
