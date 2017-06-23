cmake_minimum_required(VERSION 3.0.2)
project (POSIX)

add_definitions(-Wall -std=gnu11)

include_directories(.)

add_executable(server ./server.c)
target_link_libraries(server rt)
add_executable(client ./client.c)
target_link_libraries(client rt)