include_directories(${CMAKE_SOURCE_DIR}/includes)

set(MALLOC_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/malloc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/heap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/utils.c)

add_library(${LIBRARY_NAME} SHARED ${MALLOC_SRCS})
