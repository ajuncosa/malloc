include_directories(${CMAKE_SOURCE_DIR}/includes)

add_library(${LIBRARY_NAME} SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/malloc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/heap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/utils.c)
