cmake_minimum_required(VERSION 3.0.2)
project (Threads)

add_definitions(-Wall -std=gnu11)

add_executable(zad1a zad1a.c)
target_link_libraries(zad1a pthread)
add_executable(zad1b zad1b.c)
target_link_libraries(zad1b pthread)
add_executable(zad1c zad1c.c)
target_link_libraries(zad1c pthread)
add_executable(generator generator.c)
