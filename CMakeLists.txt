cmake_minimum_required(VERSION 3.22.1)
project(ft_malloc
    VERSION 1.0
    LANGUAGES C)

set(CMAKE_C_FLAGS "-Wall -Werror -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-g3 -fsanitize=address")

if(NOT DEFINED ENV{HOSTTYPE})
    execute_process(
        COMMAND uname -m
        OUTPUT_STRIP_TRAILING_WHITESPACE
        OUTPUT_VARIABLE MACHINE_NAME)
    execute_process(
        COMMAND uname -s
        OUTPUT_STRIP_TRAILING_WHITESPACE
        OUTPUT_VARIABLE KERNEL_NAME)
    set(HOSTTYPE ${MACHINE_NAME}_${KERNEL_NAME})
endif()

set(LIBRARY_NAME "ft_malloc_${HOSTTYPE}")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

enable_testing()

add_subdirectory(src)
add_subdirectory(test)
