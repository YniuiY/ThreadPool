cmake_minimum_required(VERSION 3.5.1)
project( "thread_pool_sample" VERSION 2022 LANGUAGES CXX C )

# add_definitions("-Wall -g")
set(CMAKE_CXX_STANDARD 14)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

include_directories(
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/../include
)

# sensor plugin lib
link_directories(
  ${PROJECT_SOURCE_DIR}/../lib
)

set(DGRAPH_TUTORIAL_LIST
  sample
  sample-steal
  sample-batch
  sample-adapt
  sample-bind-cpu
  sample-future
  sample-long_time_run
)

foreach(tut ${DGRAPH_TUTORIAL_LIST})
    add_executable(${tut} ./src/${tut}.cpp)
    target_link_libraries(${tut} thread_pool pthread)
endforeach()
