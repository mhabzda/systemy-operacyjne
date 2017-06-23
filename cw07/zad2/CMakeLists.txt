cmake_minimum_required(VERSION 3.0.2)
project (POSIX)

add_definitions(-Wall -std=gnu11)

add_executable(golibroda ./golibroda.c)
target_link_libraries(golibroda pthread)
target_link_libraries(golibroda rt)
add_executable(klient ./klient.c)
target_link_libraries(klient pthread)
target_link_libraries(klient rt)