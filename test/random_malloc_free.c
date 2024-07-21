#include <time.h>
#include "test_utils.h"
#include "malloc.h"

#include <stdio.h>

 // 1000 random mallocs with frees
int main()
{
	char *test_name = "test_random_malloc_free";
	srand(time(NULL));
	size_t in_use_tiny_chunk_size = (size_t)(getpagesize() / 128 | IN_USE);

	size_t malloc_size = 0;
	char *ptr = NULL;
	size_t *chunk_begin = NULL;
	for (int i = 0; i < 1000; i++)
	{
		malloc_size = RANDOM_IN_RANGE(0, SMALL_ZONE_MAX_CHUNK_SIZE * 2);
		ptr = malloc(malloc_size);
		if (malloc_size == 0)
		{
			ASSERT_POINTER_EQ(ptr, NULL);
			continue;
		}

		ASSERT_POINTER_NE(ptr, NULL);
		chunk_begin = (size_t *)(ptr - SIZE_T_SIZE);
		if (malloc_size <= TINY_ZONE_MAX_CHUNK_SIZE - SIZE_T_SIZE)
		{
			ASSERT_SIZE_EQ(*chunk_begin, in_use_tiny_chunk_size);
			ASSERT_SIZE_EQ(zone_list_len(heap_g.tiny_zones_head), 1);
		}
		else if (malloc_size > TINY_ZONE_MAX_CHUNK_SIZE - SIZE_T_SIZE
			&& malloc_size <= SMALL_ZONE_MAX_CHUNK_SIZE - SIZE_T_SIZE)
		{
			ASSERT_SIZE_EQ(*chunk_begin, (ALIGN(malloc_size + SIZE_T_SIZE) | IN_USE));
			ASSERT_SIZE_EQ(zone_list_len(heap_g.small_zones_head), 1);
		}
		else
		{
			ASSERT_SIZE_EQ(*chunk_begin, (ALIGN(malloc_size + SIZE_T_SIZE) | IN_USE));
			ASSERT_SIZE_EQ(zone_list_len(heap_g.large_zones_head), 1);
		}
		free(ptr);
	}

	if (heap_g.tiny_zones_head != NULL)
	{
		ASSERT_SIZE_EQ(zone_list_len(heap_g.tiny_zones_head), 1);
		ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.tiny_bin_head), NUMBER_OF_TINY_CHUNKS_PER_ZONE);
	}
	if (heap_g.small_zones_head != NULL)
	{
		ASSERT_SIZE_EQ(zone_list_len(heap_g.small_zones_head), 1);
		ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_bin_head), 1);
		ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_unsorted_list_head), 1);
	}
	ASSERT_SIZE_EQ(zone_list_len(heap_g.large_zones_head), 0);

	pass_test(test_name);
}
