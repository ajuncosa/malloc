#include <stdio.h>
#include "tests.h"
#include "malloc.h"

// coalescing
int main()
{
	char *test_name = "test_small_malloc_5";
    size_t malloc_1_size = 10000;
    size_t malloc_2_size = 20000;
    size_t malloc_3_size = 10000;
    size_t malloc_4_size = 10000;
    size_t malloc_5_size = 20000;

	char *ptr1 = malloc(malloc_1_size);
	ASSERT_POINTER_NE(ptr1, NULL);

	char *ptr2 = malloc(malloc_2_size);
	ASSERT_POINTER_NE(ptr2, NULL);

    char *ptr3 = malloc(malloc_3_size);
	ASSERT_POINTER_NE(ptr3, NULL);

    char *ptr4 = malloc(malloc_4_size);
	ASSERT_POINTER_NE(ptr3, NULL);

	ASSERT_POINTER_EQ(heap_g.tiny_zones_head, NULL);
	ASSERT_POINTER_NE(heap_g.small_zones_head, NULL);
	ASSERT_POINTER_EQ(heap_g.large_zones_head, NULL);
	ASSERT_POINTER_EQ(heap_g.tiny_bin_head, NULL);
	ASSERT_POINTER_NE(heap_g.small_bin_head, NULL);
	ASSERT_POINTER_EQ(heap_g.small_unsorted_list_head, NULL);

	ASSERT_SIZE_EQ(zone_list_len(heap_g.small_zones_head), 1);
	ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_bin_head), 1);
	ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_unsorted_list_head), 0);

	size_t *chunk1_begin = (size_t *)((char *)ptr1 - SIZE_T_SIZE);
	ASSERT_SIZE_EQ(*chunk1_begin, (ALIGN(malloc_1_size + SIZE_T_SIZE) | IN_USE));

	size_t *chunk2_begin = (size_t *)((char *)ptr2 - SIZE_T_SIZE);
	ASSERT_SIZE_EQ(*chunk2_begin, (ALIGN(malloc_2_size + SIZE_T_SIZE) | IN_USE));

	size_t *chunk3_begin = (size_t *)((char *)ptr3 - SIZE_T_SIZE);
	ASSERT_SIZE_EQ(*chunk3_begin, (ALIGN(malloc_3_size + SIZE_T_SIZE) | IN_USE));

	size_t *chunk4_begin = (size_t *)((char *)ptr4 - SIZE_T_SIZE);
	ASSERT_SIZE_EQ(*chunk4_begin, (ALIGN(malloc_4_size + SIZE_T_SIZE) | IN_USE));

	ASSERT_NO_CHUNK_OVERLAP(chunk1_begin, chunk2_begin);
	ASSERT_NO_CHUNK_OVERLAP(chunk2_begin, chunk3_begin);
	ASSERT_NO_CHUNK_OVERLAP(chunk3_begin, chunk4_begin);
	ASSERT_NO_CHUNK_OVERLAP(chunk1_begin, chunk4_begin);

	free(ptr1);
	free(ptr2);
	free(ptr3);
	free(ptr4);

    ASSERT_SIZE_EQ(zone_list_len(heap_g.small_zones_head), 1);
	ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_bin_head), 1);
	ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_unsorted_list_head), 4);

    char *ptr5 = malloc(malloc_5_size);
    char *ptr6 = malloc(malloc_5_size);
    (void)ptr6;
	ASSERT_POINTER_NE(ptr5, NULL);

	size_t *chunk5_begin = (size_t *)((char *)ptr5 - SIZE_T_SIZE);
	ASSERT_SIZE_EQ(*chunk5_begin, (ALIGN(malloc_5_size + SIZE_T_SIZE) | IN_USE));

	ASSERT_SIZE_EQ(zone_list_len(heap_g.small_zones_head), 1);
	ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_bin_head), 1);
	ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_unsorted_list_head), 0);
	
    free(ptr5);

	ASSERT_SIZE_EQ(zone_list_len(heap_g.small_zones_head), 1);
	ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_bin_head), 1);
	ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_unsorted_list_head), 1);
	
	pass_test(test_name);
}
