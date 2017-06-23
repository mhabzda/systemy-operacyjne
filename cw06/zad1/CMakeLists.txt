cmake_minimum_required(VERSION 3.0.2)
project (IPC)

add_definitions(-Wall -std=gnu11)

include_directories(.)

add_executable(server ./server.c)
add_executable(client ./client.c)
