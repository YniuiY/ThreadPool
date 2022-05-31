cmake_minimum_required(VERSION 3.1)

project(ThreadPool)

include_directories(./include)

aux_source_directory(./src SRCS)

add_executable(run ${SRCS})

add_library(ThreadPool SHARED ${SRCS})

target_link_libraries(run thread)