cmake_minimum_required(VERSION 3.0.2)
project (TestLibrary)

add_definitions(-Wall)
include_directories(../zad1/)
link_directories(../zad1/build/)

set ( SHARED libContactBook.so )
add_executable(test_shared ../zad2/test.c)
target_link_libraries(test_shared ${SHARED} )

set (STATIC libContactBookStatic.a)
#link_directories(../zad1/build/)
add_executable(test_static ../zad2/test.c)
target_link_libraries(test_static ${STATIC} )

add_executable(test_dynamic test_dl.c)
target_link_libraries(test_dynamic dl)
target_compile_options(test_dynamic PRIVATE -ldl)
