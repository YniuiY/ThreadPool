cmake_minimum_required(VERSION 3.1)

project(ThreadPool)

# add_definitions("-Wall -g")

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

include_directories(./include)

aux_source_directory(./src SRCS)
aux_source_directory(./src/reject_policy REJECT_SRCS)
aux_source_directory(./src/thread/ THREAD_SRCS)

add_library(thread_pool SHARED ${SRCS} ${REJECT_SRCS} ${THREAD_SRCS})

target_link_libraries(thread_pool pthread)