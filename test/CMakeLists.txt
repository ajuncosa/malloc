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
        COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/random_tester/run_random_test.sh ${PROJECT_BINARY_DIR}/bin ${commands_file} ${n_allocations} ${n_reallocations} ${lower_bound} ${upper_bound} ${initial_offset})
endfunction()

add_executable(rand_test
    ${CMAKE_CURRENT_SOURCE_DIR}/random_tester/rand-tester.c
    ${CMAKE_CURRENT_SOURCE_DIR}/test_utils.c)
target_link_libraries(rand_test PRIVATE ${LIBRARY_NAME})

set(MAX_TINY_CHUNK_SIZE 16)
set(MIN_SMALL_CHUNK_SIZE 17)
set(MAX_SMALL_CHUNK_SIZE 32752)
set(MIN_LARGE_CHUNK_SIZE 32753)

# tiny random tests
create_random_test("test_rand_tiny_1" 50 0 0 ${MAX_TINY_CHUNK_SIZE} 20)
create_random_test("test_rand_tiny_2" 100 50 0 ${MAX_TINY_CHUNK_SIZE} 30)
create_random_test("test_rand_tiny_3" 200 100 0 ${MAX_TINY_CHUNK_SIZE} 50)
create_random_test("test_rand_tiny_4" 400 200 0 ${MAX_TINY_CHUNK_SIZE} 70)
create_random_test("test_rand_tiny_5" 800 500 0 ${MAX_TINY_CHUNK_SIZE} 100)
create_random_test("test_rand_tiny_6" 1600 0 0 ${MAX_TINY_CHUNK_SIZE} 300)
create_random_test("test_rand_tiny_7" 1600 300 0 ${MAX_TINY_CHUNK_SIZE} 300)
create_random_test("test_rand_tiny_8" 2000 800 0 ${MAX_TINY_CHUNK_SIZE} 400)

# small random tests
create_random_test("test_rand_small_1" 50 0 ${MIN_SMALL_CHUNK_SIZE} ${MAX_SMALL_CHUNK_SIZE} 20)
create_random_test("test_rand_small_2" 100 0 ${MIN_SMALL_CHUNK_SIZE} ${MAX_SMALL_CHUNK_SIZE} 40)
create_random_test("test_rand_small_3" 200 0 ${MIN_SMALL_CHUNK_SIZE} ${MAX_SMALL_CHUNK_SIZE} 50)
create_random_test("test_rand_small_4" 200 0 ${MIN_SMALL_CHUNK_SIZE} ${MAX_SMALL_CHUNK_SIZE} 100)
create_random_test("test_rand_small_5" 600 0 ${MIN_SMALL_CHUNK_SIZE} ${MAX_SMALL_CHUNK_SIZE} 100)
create_random_test("test_rand_small_6" 600 0 ${MIN_SMALL_CHUNK_SIZE} ${MAX_SMALL_CHUNK_SIZE} 200)
create_random_test("test_rand_small_7" 600 100 ${MIN_SMALL_CHUNK_SIZE} ${MAX_SMALL_CHUNK_SIZE} 200)
create_random_test("test_rand_small_8" 1000 300 ${MIN_SMALL_CHUNK_SIZE} ${MAX_SMALL_CHUNK_SIZE} 400)
create_random_test("test_rand_small_9" 2000 800 ${MIN_SMALL_CHUNK_SIZE} ${MAX_SMALL_CHUNK_SIZE} 500)
create_random_test("test_rand_small_10" 10000 5000 ${MIN_SMALL_CHUNK_SIZE} ${MAX_SMALL_CHUNK_SIZE} 1000)

# large random tests
create_random_test("test_rand_large_1" 50 30 ${MIN_LARGE_CHUNK_SIZE} 1000000 30)
create_random_test("test_rand_large_2" 100 50 ${MIN_LARGE_CHUNK_SIZE} 1000000 40)
create_random_test("test_rand_large_3" 200 50 ${MIN_LARGE_CHUNK_SIZE} 1000000 50)
create_random_test("test_rand_large_4" 200 50 ${MIN_LARGE_CHUNK_SIZE} 1000000 100)
create_random_test("test_rand_large_5" 600 50 ${MIN_LARGE_CHUNK_SIZE} 1000000 100)
create_random_test("test_rand_large_6" 600 50 ${MIN_LARGE_CHUNK_SIZE} 1000000 200)
create_random_test("test_rand_large_7" 600 100 ${MIN_LARGE_CHUNK_SIZE} 1000000 200)
create_random_test("test_rand_large_8" 1000 300 ${MIN_LARGE_CHUNK_SIZE} 1000000 400)
create_random_test("test_rand_large_9" 2000 800 ${MIN_LARGE_CHUNK_SIZE} 1000000 500)
create_random_test("test_rand_large_10" 10000 5000 ${MIN_LARGE_CHUNK_SIZE} 1000000 1000)

# mixed random tests
create_random_test("test_rand_mix_1" 2000 900 0 1000 500)
create_random_test("test_rand_mix_2" 2000 900 100 200000 500)
create_random_test("test_rand_mix_3" 2000 900 0 1000000 500)
create_random_test("test_rand_mix_4" 10000 1000 0 2000000 800)
create_random_test("test_rand_mix_5" 10000 1000 0 ${MIN_LARGE_CHUNK_SIZE} 800)
