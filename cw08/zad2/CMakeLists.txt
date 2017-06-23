cmake_minimum_required(VERSION 3.0.2)
project (Threads)

add_definitions(-Wall -std=gnu11)

add_executable(zad2a zad2a.c)
target_link_libraries(zad2a pthread)
add_executable(zad2b zad2b.c)
target_link_libraries(zad2b pthread)
