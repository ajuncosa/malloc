include_directories(${CMAKE_SOURCE_DIR}/includes)

function(create_test name)
    add_executable(test_${name}
        ${CMAKE_CURRENT_SOURCE_DIR}/${name}.c
        ${CMAKE_CURRENT_SOURCE_DIR}/test_utils.c)
    target_link_libraries(test_${name} PRIVATE ${LIBRARY_NAME})
    add_test(NAME test_${name} COMMAND test_${name})
endfunction()

# init
create_test(init)

# malloc
create_test(tiny_malloc_1)
create_test(tiny_malloc_2)
create_test(tiny_malloc_3)
create_test(tiny_malloc_4)
create_test(small_malloc_1)
create_test(small_malloc_2)
create_test(small_malloc_3)
create_test(small_malloc_4)
create_test(small_malloc_5)
create_test(large_malloc_1)
create_test(large_malloc_2)

# realloc
create_test(tiny_realloc_1)
create_test(tiny_realloc_2)
create_test(small_realloc_1)
create_test(small_realloc_2)
create_test(small_realloc_3)
create_test(small_realloc_4)
create_test(small_realloc_5)
create_test(large_realloc_1)

# random
create_test(random_malloc)
create_test(random_malloc_free)
