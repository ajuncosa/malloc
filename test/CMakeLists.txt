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

# RANDOM TESTER

function(create_random_test name n_allocations n_reallocations lower_bound upper_bound initial_offset)
    set(commands_file "${name}_commands")
    add_test(NAME ${name}
        COMMAND bash ${CMAKE_CURRENT_BINARY_DIR}/random_tester/run_random_test.sh ${PROJECT_BINARY_DIR}/bin ${commands_file} ${n_allocations} ${n_reallocations} ${lower_bound} ${upper_bound} ${initial_offset})
endfunction()

add_executable(rand_test ${CMAKE_CURRENT_SOURCE_DIR}/random_tester/rand-tester.c)
target_link_libraries(rand_test PRIVATE ${LIBRARY_NAME})

# tiny random tests
create_random_test("test_rand_tiny_1" 100 0 0 120 30)
create_random_test("test_rand_tiny_2" 200 0 1 120 50)
create_random_test("test_rand_tiny_3" 400 0 1 120 70)
create_random_test("test_rand_tiny_4" 800 0 1 120 100)
create_random_test("test_rand_tiny_5" 1600 0 1 120 300)
create_random_test("test_rand_tiny_6" 1600 300 1 120 300)
create_random_test("test_rand_tiny_8" 2000 800 1 120 400)

# small random tests
create_random_test("test_rand_small_1" 50 0 130 60000 20)
create_random_test("test_rand_small_2" 100 0 130 60000 40)
create_random_test("test_rand_small_3" 200 0 130 60000 50)
create_random_test("test_rand_small_4" 200 0 130 60000 100)
create_random_test("test_rand_small_5" 600 0 130 60000 100)
create_random_test("test_rand_small_6" 600 0 130 60000 200)
create_random_test("test_rand_small_7" 1000 0 130 60000 400)
create_random_test("test_rand_small_8" 10000 0 130 60000 1000)

# large random tests
create_random_test("test_rand_large_1" 50 30 130 60000 30)
