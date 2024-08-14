#include <time.h>
#include <stdint.h>
#include "test_utils.h"
#include "malloc.h"

 // 1000 random mallocs with frees
int main()
{
	srand(time(NULL));
	size_t in_use_tiny_chunk_size = (size_t)(TINY_ZONE_MAX_CHUNK_SIZE | IN_USE);

	size_t malloc_size = 0;
	uint8_t malloc_size_type = 0; // to distribute between tiny, small and large
	char *ptr = NULL;
	size_t *chunk_begin = NULL;
	for (int i = 0; i < 1000; i++)
	{
		malloc_size_type = RANDOM_IN_RANGE(0, 2);
		if (malloc_size_type == 0)
			malloc_size = RANDOM_IN_RANGE(0, TINY_ZONE_MAX_CHUNK_SIZE - SIZE_T_SIZE);
		else if (malloc_size_type == 1)
			malloc_size = RANDOM_IN_RANGE(TINY_ZONE_MAX_CHUNK_SIZE - SIZE_T_SIZE + 1, SMALL_ZONE_MAX_CHUNK_SIZE - SIZE_T_SIZE);
		else
			malloc_size = RANDOM_IN_RANGE(SMALL_ZONE_MAX_CHUNK_SIZE - SIZE_T_SIZE + 1, SIZE_MAX - SIZE_T_SIZE - 1);

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
}
