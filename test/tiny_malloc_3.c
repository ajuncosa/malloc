#include "test_utils.h"
#include "malloc.h"

// NUMBER_OF_TINY_CHUNKS_PER_ZONE tiny mallocs
int main()
{
	size_t in_use_tiny_chunk_size = (size_t)(TINY_ZONE_MAX_CHUNK_SIZE | IN_USE);

	size_t *prev_chunk_begin = NULL;
	char *ptr = NULL;
	for (size_t i = 0; i < NUMBER_OF_TINY_CHUNKS_PER_ZONE; i++)
	{
		ptr = malloc(1);
		ASSERT_POINTER_NE(ptr, NULL);
		ASSERT_ALIGNMENT(ptr);

		size_t *chunk_begin = (size_t *)((char *)ptr - SIZE_T_SIZE);
		ASSERT_SIZE_EQ(*chunk_begin, in_use_tiny_chunk_size);
		ASSERT_NO_CHUNK_OVERLAP(chunk_begin, prev_chunk_begin);
		prev_chunk_begin = chunk_begin;
	}

	ASSERT_POINTER_NE(heap_g.tiny_zones_head, NULL);
	ASSERT_POINTER_EQ(heap_g.small_zones_head, NULL);
	ASSERT_POINTER_EQ(heap_g.large_zones_head, NULL);
	ASSERT_POINTER_EQ(heap_g.tiny_bin_head, NULL);
	ASSERT_POINTER_EQ(heap_g.small_bin_head, NULL);
	ASSERT_POINTER_EQ(heap_g.small_unsorted_list_head, NULL);

	ASSERT_SIZE_EQ(zone_list_len(heap_g.tiny_zones_head), 1);
	ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.tiny_bin_head), 0);

	for (size_t i = 0; i < NUMBER_OF_TINY_CHUNKS_PER_ZONE; i++)
	{
		free(ptr);
		ptr -= heap_g.tiny_zone_chunk_max_size;
	}

	ASSERT_SIZE_EQ(zone_list_len(heap_g.tiny_zones_head), 1);
	ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.tiny_bin_head), NUMBER_OF_TINY_CHUNKS_PER_ZONE);
}
