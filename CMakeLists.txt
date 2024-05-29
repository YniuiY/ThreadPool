cmake_minimum_required(VERSION 3.1)

project(ThreadPool)

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
  # add_definitions("-Wall -O2")
ENDIF()

# add_definitions("-Wall -g")
option(INSTALL "install" ON)

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

include_directories(./include)

aux_source_directory(./src SRCS)
aux_source_directory(./src/reject_policy REJECT_SRCS)
aux_source_directory(./src/thread/ THREAD_SRCS)

add_library(thread_pool SHARED ${SRCS} ${REJECT_SRCS} ${THREAD_SRCS})

target_link_libraries(thread_pool pthread)

if(INSTALL)
  set(OUTPUT "${PROJECT_SOURCE_DIR}/output/ThreadPool")
  message("install to ${OUTPUT}")
  install(TARGETS thread_pool LIBRARY DESTINATION ${OUTPUT}/lib)
  install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION ${OUTPUT}/include)
endif()