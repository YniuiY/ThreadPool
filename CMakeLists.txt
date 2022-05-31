cmake_minimum_required(VERSION 3.1)

project(ThreadPool)

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR})

include_directories(./include)

aux_source_directory(./src SRCS)

add_executable(run ${SRCS})

add_library(ThreadPool SHARED ${SRCS})

target_link_libraries(run pthread)