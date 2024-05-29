cmake_minimum_required(VERSION 3.5.1)
project( "thread_pool_sample" VERSION 2022 LANGUAGES CXX C )

IF(DEFINED COMPILE_TOOLCHAIN_ROOT)
  message("compile ARM, targetf ${COMPILE_TOOLCHAIN_ROOT}")
  add_definitions("-O2")
  set(TEST_COMPILER_ROOT "${COMPILE_TOOLCHAIN_ROOT}/gcc-linaro-6.5.0-2018.12-x86_64_aarch64-linux-gnu/bin")

  set(CMAKE_SYSTEM_NAME Linux)
  set(CMAKE_SYSTEM_PROCESSOR arm)

  set(CMAKE_CXX_COMPILER ${TEST_COMPILER_ROOT}/aarch64-linux-gnu-g++)
  set(CMAKE_C_COMPILER ${TEST_COMPILER_ROOT}/aarch64-linux-gnu-gcc)

  set(CMAKE_CXX_STANDARD 14)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -Wall -g2 -ggdb")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -Wall")
ELSEIF(DEFINED COMPILE_TOOLCHAIN_ROOT_10_3_1)
  set(TOOLCHAIN_COMPILER "gcc-10.3-2021.07-x86_64-aarch64-none-linux-gnu")
  message(STATUS "Compile ARM, targetfs ${COMPILE_TOOLCHAIN_ROOT_10_3_1}")
  message(STATUS "Compile ARM, toolchain ${TOOLCHAIN_COMPILER}")
  add_definitions("-O2")
  set(TOOLCHAIN_COMPILER_ROOT "${COMPILE_TOOLCHAIN_ROOT_10_3_1}/${TOOLCHAIN_COMPILER}/bin")

  set(CMAKE_SYSTEM_NAME Linux)
  set(CMAKE_SYSTEM_PROCESSOR arm)
  set(CMAKE_SYSTEM_PLATFORM aarch64)
  set(CMAKE_GCC_VERSION 10.3.1)

  set(CMAKE_CXX_COMPILER ${TOOLCHAIN_COMPILER_ROOT}/aarch64-none-linux-gnu-g++)
  set(CMAKE_C_COMPILER ${TOOLCHAIN_COMPILER_ROOT}/aarch64-none-linux-gnu-gcc)

  set(CMAKE_CXX_STANDARD 14)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -Wall -g2 -ggdb")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -Wall")
ELSEIF(DEFINED COMPILE_TOOLCHAIN_ROOT_12_1_1)
  set(TOOLCHAIN_COMPILER "gcc-12.1-2022.08-cortex-a53-aarch64-linux5.15")
  message(STATUS "Compile ARM, targetfs ${COMPILE_TOOLCHAIN_ROOT_12_1_1}")
  message(STATUS "Compile ARM, toolchain ${TOOLCHAIN_COMPILER}")
  set(CMAKE_BUILD_TYPE Release)
  # set(CMAKE_BUILD_TYPE Debug)
  set(TOOLCHAIN_COMPILER_ROOT "${COMPILE_TOOLCHAIN_ROOT_12_1_1}/${TOOLCHAIN_COMPILER}/bin")

  set(CMAKE_SYSTEM_NAME Linux)
  set(CMAKE_SYSTEM_PROCESSOR arm)

  set(CMAKE_CXX_COMPILER ${TOOLCHAIN_COMPILER_ROOT}/aarch64-linux-gnu-g++)
  set(CMAKE_C_COMPILER ${TOOLCHAIN_COMPILER_ROOT}/aarch64-linux-gnu-gcc)

  set(CMAKE_CXX_STANDARD 14)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -Wall -g2 -ggdb")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -Wall")
ELSE()
  # add_definitions("-Wall -g")
  add_definitions("-Wall -O2")
ENDIF()

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
