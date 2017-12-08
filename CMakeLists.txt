cmake_minimum_required(VERSION 3.8)

project("wwsi-pp")
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

configure_file("data/rawstats" "bin/rawstats" COPYONLY)
configure_file("data/rawstats2" "bin/rawstats2" COPYONLY)

add_executable("wwsi-pp" "main.cpp")