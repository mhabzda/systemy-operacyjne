#zad3b
cmake_minimum_required(VERSION 3.0.2)
project (TestLibrary)

add_definitions(-Wall)
include_directories(../zad1/)
link_directories(../zad1/build/)

set ( SHARED libContactBook.so )

#SHARED
add_executable(test_shared00 ../zad2/test.c)
target_link_libraries(test_shared00 ${SHARED} )
target_compile_options(test_shared00 PRIVATE -O0 )

add_executable(test_shared01 ../zad2/test.c)
target_link_libraries(test_shared01 ${SHARED} )
target_compile_options(test_shared01 PRIVATE -O1 )

add_executable(test_shared02 ../zad2/test.c)
target_link_libraries(test_shared02 ${SHARED} )
target_compile_options(test_shared02 PRIVATE -O2 )

add_executable(test_shared03 ../zad2/test.c)
target_link_libraries(test_shared03 ${SHARED} )
target_compile_options(test_shared03 PRIVATE -O3 )

add_executable(test_shared0s ../zad2/test.c)
target_link_libraries(test_shared0s ${SHARED} )
target_compile_options(test_shared0s PRIVATE -Os )

set (STATIC libContactBookStatic.a)

#STATIC
add_executable(test_static00 ../zad2/test.c)
target_link_libraries(test_static00 ${STATIC} )
target_compile_options(test_static00 PRIVATE -O0 )

add_executable(test_static01 ../zad2/test.c)
target_link_libraries(test_static01 ${STATIC} )
target_compile_options(test_static01 PRIVATE -O1 )

add_executable(test_static02 ../zad2/test.c)
target_link_libraries(test_static02 ${STATIC} )
target_compile_options(test_static02 PRIVATE -O2 )

add_executable(test_static03 ../zad2/test.c)
target_link_libraries(test_static03 ${STATIC} )
target_compile_options(test_static03 PRIVATE -O3 )

add_executable(test_static0s ../zad2/test.c)
target_link_libraries(test_static0s ${STATIC} )
target_compile_options(test_static0s PRIVATE -Os )

#DYNAMIC
add_executable(test_dynamic00 ../zad3a/test_dl.c)
target_link_libraries(test_dynamic00 dl)
target_compile_options(test_dynamic00 PRIVATE -ldl -O0)

add_executable(test_dynamic01 ../zad3a/test_dl.c)
target_link_libraries(test_dynamic01 dl)
target_compile_options(test_dynamic01 PRIVATE -ldl -O1)

add_executable(test_dynamic02 ../zad3a/test_dl.c)
target_link_libraries(test_dynamic02 dl)
target_compile_options(test_dynamic02 PRIVATE -ldl -O2)

add_executable(test_dynamic03 ../zad3a/test_dl.c)
target_link_libraries(test_dynamic03 dl)
target_compile_options(test_dynamic03 PRIVATE -ldl -O3)

add_executable(test_dynamic0s ../zad3a/test_dl.c)
target_link_libraries(test_dynamic0s dl)
target_compile_options(test_dynamic0s PRIVATE -ldl -Os)









