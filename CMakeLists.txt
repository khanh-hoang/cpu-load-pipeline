cmake_minimum_required(VERSION 3.20)

# Cross-compile for the i.MX 8M Plus target by default.
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/imx8mp-toolchain.cmake)
endif()

project(thesis-pipeline LANGUAGES C)

# Use optimized builds for calibration and experiment runs.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_executable(thesis-pipeline
  src/main.c
  src/pipeline_config.c
  src/pipeline_threads.c
  src/job_queue.c
  src/sched_deadline.c
  src/sched_fifo.c
  src/sched_affinity.c
  src/workload.c
  src/timing_utils.c
  src/csv_writer.c
)

target_compile_options(thesis-pipeline PRIVATE -Wall -Wextra)

find_package(Threads REQUIRED)
target_link_libraries(thesis-pipeline PRIVATE Threads::Threads)
