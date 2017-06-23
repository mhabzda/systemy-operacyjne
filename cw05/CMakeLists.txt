cmake_minimum_required(VERSION 3.0.2)
project (Pipes)

add_definitions(-Wall -std=gnu11)

add_executable(zad1 ./zad1/zad1)

add_executable(slave ./zad2/slave.c)
target_link_libraries(slave m)
add_executable(master ./zad2/master.c)
