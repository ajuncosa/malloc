#include <time.h>
#include <stdint.h>
#include "test_utils.h"
#include "malloc.h"

 // 1000 random mallocs no frees
int main()
{
	srand(time(NULL));
	size_t in_use_tiny_chunk_size = (size_t)(TINY_ZONE_MAX_CHUNK_SIZE | IN_USE);
	size_t tiny_malloced_bytes = 0;
	size_t small_malloced_bytes = 0;
	size_t number_of_tiny_zones = 1;
	size_t number_of_small_zones = 1;
	size_t number_of_large_zones = 0;

	size_t malloc_size = 0;
	uint8_t malloc_size_type = 0; // to distribute between tiny, small and large
	char *ptr = NULL;
	size_t *chunk_begin = NULL;
	size_t *prev_chunk_begin = NULL;
	for (int i = 0; i < 1000; i++)
	{
		malloc_size_type = RANDOM_IN_RANGE(0, 2);
		if (malloc_size_type == 0)
			malloc_size = RANDOM_IN_RANGE(0, TINY_ZONE_MAX_CHUNK_SIZE - SIZE_T_SIZE);
		else if (malloc_size_type == 1)
			malloc_size = RANDOM_IN_RANGE(TINY_ZONE_MAX_CHUNK_SIZE - SIZE_T_SIZE + 1, SMALL_ZONE_MAX_CHUNK_SIZE - SIZE_T_SIZE);
		else
			malloc_size = RANDOM_IN_RANGE(SMALL_ZONE_MAX_CHUNK_SIZE - SIZE_T_SIZE + 1, SIZE_MAX - SIZE_T_SIZE - 1);
		
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
			tiny_malloced_bytes += CHUNK_SIZE_WITHOUT_FLAGS(*chunk_begin);
			if (tiny_malloced_bytes >= heap_g.tiny_zone_size)
			{
				number_of_tiny_zones++;
				tiny_malloced_bytes = CHUNK_SIZE_WITHOUT_FLAGS(*chunk_begin);
			}
			ASSERT_SIZE_EQ(zone_list_len(heap_g.tiny_zones_head), number_of_tiny_zones);
		}
		else if (malloc_size > TINY_ZONE_MAX_CHUNK_SIZE - SIZE_T_SIZE
			&& malloc_size <= SMALL_ZONE_MAX_CHUNK_SIZE - SIZE_T_SIZE)
		{
			ASSERT_SIZE_EQ(*chunk_begin, (ALIGN(malloc_size + SIZE_T_SIZE) | IN_USE));
			small_malloced_bytes += CHUNK_SIZE_WITHOUT_FLAGS(*chunk_begin);
			if (heap_g.small_zone_size < small_malloced_bytes
				|| heap_g.small_zone_size - small_malloced_bytes <= TINY_ZONE_MAX_CHUNK_SIZE - SIZE_T_SIZE)
			{
				number_of_small_zones++;
				small_malloced_bytes = CHUNK_SIZE_WITHOUT_FLAGS(*chunk_begin);
			}
			ASSERT_SIZE_EQ(zone_list_len(heap_g.small_zones_head), number_of_small_zones);
		}
		else
		{
			ASSERT_SIZE_EQ(*chunk_begin, (ALIGN(malloc_size + SIZE_T_SIZE) | IN_USE));
			number_of_large_zones++;
			ASSERT_SIZE_EQ(zone_list_len(heap_g.large_zones_head), number_of_large_zones);
		}
		ASSERT_NO_CHUNK_OVERLAP(chunk_begin, prev_chunk_begin);
		prev_chunk_begin = chunk_begin;
	}
}
