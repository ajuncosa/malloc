#include <stdio.h>
#include "tests.h"
#include "malloc.h"

// tiny to small
int main()
{
	char *test_name = "test_tiny_realloc_2";

	char *ptr = malloc(50);
	ASSERT_POINTER_NE(ptr, NULL);
	for(int i = 0; i < 50; i++)
		ptr[i] = 'a' + i % 26;

	ASSERT_SIZE_EQ(zone_list_len(heap_g.tiny_zones_head), 1);
	ASSERT_SIZE_EQ(zone_list_len(heap_g.small_zones_head), 0);
	ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.tiny_bin_head), NUMBER_OF_TINY_CHUNKS_PER_ZONE - 1);
    
    char *new_ptr = realloc(ptr, 200);
	ASSERT_POINTER_NE(new_ptr, NULL);
	ASSERT_POINTER_NE(new_ptr, ptr);

	ASSERT_SIZE_EQ(zone_list_len(heap_g.tiny_zones_head), 1);
	ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.tiny_bin_head), NUMBER_OF_TINY_CHUNKS_PER_ZONE);
	ASSERT_SIZE_EQ(zone_list_len(heap_g.small_zones_head), 1);
	ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_bin_head), 1);
	ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_unsorted_list_head), 0);

	for(int i = 0; i < 50; i++)
		ASSERT_CHAR_EQ(new_ptr[i], 'a' + i % 26);

	free(new_ptr);

	ASSERT_SIZE_EQ(zone_list_len(heap_g.tiny_zones_head), 1);
	ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.tiny_bin_head), NUMBER_OF_TINY_CHUNKS_PER_ZONE);
	ASSERT_SIZE_EQ(zone_list_len(heap_g.small_zones_head), 1);
	ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_bin_head), 1);
	ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_unsorted_list_head), 1);

	pass_test(test_name);
}
