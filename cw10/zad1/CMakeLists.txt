cmake_minimum_required(VERSION 3.0.2)
project (Socket)

add_definitions(-Wall -std=gnu11)

add_executable(server server.c)
target_link_libraries(server pthread)
add_executable(client client.c)