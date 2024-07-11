#include <stdio.h>
#include "tests.h"
#include "malloc.h"

// MIN_NUMBER_OF_SMALL_CHUNKS_PER_ZONE + 1 small mallocs
void test_small_malloc_4()
{
	char *test_name = "test_small_malloc_4";
	char error_reason[ERROR_MSG_MAX_LEN];
    size_t malloc_size = SMALL_ZONE_MAX_CHUNK_SIZE - SIZE_T_SIZE;

	size_t *prev_chunk_begin = NULL;
	char *ptr = NULL;
	for (size_t i = 0; i < MIN_NUMBER_OF_SMALL_CHUNKS_PER_ZONE + 1; i++)
	{
		ptr = malloc(malloc_size);
		if (ptr == NULL)
        {
            snprintf(error_reason, ERROR_MSG_MAX_LEN, "malloc(%zu) returned NULL\n", malloc_size);
            fail_test(test_name, error_reason);
        }
		size_t *chunk_begin = (size_t *)((char *)ptr - SIZE_T_SIZE);
		if (*chunk_begin != (ALIGN(malloc_size + SIZE_T_SIZE) | IN_USE))
		{
			snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected small chunk size for address %p (expected: %zu, actual: %zu)\n", ptr, (ALIGN(malloc_size + SIZE_T_SIZE) | IN_USE), *chunk_begin);
			fail_test(test_name, error_reason);
		}
		if (prev_chunk_begin != NULL && overlaps(chunk_begin, prev_chunk_begin))
		{
			size_t *chunk_end = (size_t *)((char *)chunk_begin + CHUNK_SIZE_WITHOUT_FLAGS(*chunk_begin));
			size_t *prev_chunk_end = (size_t *)((char *)prev_chunk_begin + CHUNK_SIZE_WITHOUT_FLAGS(*prev_chunk_begin));
			snprintf(error_reason, ERROR_MSG_MAX_LEN, "memory at [%p - %p] and [%p - %p] is overlapping)\n", chunk_begin, chunk_end, prev_chunk_begin, prev_chunk_end);
			fail_test(test_name, error_reason);
		}
		prev_chunk_begin = chunk_begin;
	}

	size_t small_zones_len = zone_list_len(heap_g.small_zones_head);
	if (small_zones_len != 2)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of small zones after %zu small mallocs (expected: %d, actual: %zu)\n", MIN_NUMBER_OF_SMALL_CHUNKS_PER_ZONE + 1, 2, small_zones_len);
		fail_test(test_name, error_reason);
	}
	size_t small_bin_len = free_chunk_list_len(heap_g.small_bin_head);
	if (small_bin_len != 1)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of free chunks in small bin after %zu small mallocs (expected: %zu, actual: %zu)\n", MIN_NUMBER_OF_SMALL_CHUNKS_PER_ZONE + 1, (heap_g.small_zone_size * 2) - (MIN_NUMBER_OF_SMALL_CHUNKS_PER_ZONE + 1), small_bin_len);
		fail_test(test_name, error_reason);
	}
	if (CHUNK_SIZE_WITHOUT_FLAGS(heap_g.small_bin_head->size) != heap_g.small_zone_size - ZONE_HEADER_T_SIZE - (ALIGN(malloc_size + SIZE_T_SIZE)))
    {
        snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected size of the leftover free chunk in the small bin after %zu small mallocs (expected: %zu, actual: %zu)\n", MIN_NUMBER_OF_SMALL_CHUNKS_PER_ZONE + 1, heap_g.small_zone_size - ZONE_HEADER_T_SIZE - (ALIGN(malloc_size + SIZE_T_SIZE)), CHUNK_SIZE_WITHOUT_FLAGS(heap_g.small_bin_head->size));
		fail_test(test_name, error_reason);
    }
    size_t small_unsorted_list_len = free_chunk_list_len(heap_g.small_unsorted_list_head);
	if (small_unsorted_list_len != 0)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of free chunks in small unsorted list after %zu small mallocs (expected: %d, actual: %zu)\n", MIN_NUMBER_OF_SMALL_CHUNKS_PER_ZONE + 1, 0, small_unsorted_list_len);
		fail_test(test_name, error_reason);
	}

	for (size_t i = 0; i < MIN_NUMBER_OF_SMALL_CHUNKS_PER_ZONE + 1; i++)
	{
		free(ptr);
		ptr -= heap_g.small_zone_chunk_max_size;
	}

	small_zones_len = zone_list_len(heap_g.small_zones_head);
	if (small_zones_len != 2)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of small zones after %zu small frees (expected: %d, actual: %zu)\n", MIN_NUMBER_OF_SMALL_CHUNKS_PER_ZONE + 1, 2, small_zones_len);
		fail_test(test_name, error_reason);
	}
	small_bin_len = free_chunk_list_len(heap_g.small_bin_head);
	if (small_bin_len != 1)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of free chunks in small bin after %zu small frees (expected: %d, actual: %zu)\n", MIN_NUMBER_OF_SMALL_CHUNKS_PER_ZONE + 1, 1, small_bin_len);
		fail_test(test_name, error_reason);
	}
	if (CHUNK_SIZE_WITHOUT_FLAGS(heap_g.small_bin_head->size) != heap_g.small_zone_size - ZONE_HEADER_T_SIZE - (ALIGN(malloc_size + SIZE_T_SIZE)))
    {
        snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected size of the leftover free chunk in the small bin after %zu small frees (expected: %zu, actual: %zu)\n", MIN_NUMBER_OF_SMALL_CHUNKS_PER_ZONE + 1, heap_g.small_zone_size - ZONE_HEADER_T_SIZE - (ALIGN(malloc_size + SIZE_T_SIZE)), CHUNK_SIZE_WITHOUT_FLAGS(heap_g.small_bin_head->size));
		fail_test(test_name, error_reason);
    }
    small_unsorted_list_len = free_chunk_list_len(heap_g.small_unsorted_list_head);
	if (small_unsorted_list_len != MIN_NUMBER_OF_SMALL_CHUNKS_PER_ZONE)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of free chunks in small unsorted list after %zu small mallocs (expected: %zu, actual: %zu)\n", MIN_NUMBER_OF_SMALL_CHUNKS_PER_ZONE, MIN_NUMBER_OF_SMALL_CHUNKS_PER_ZONE, small_unsorted_list_len);
		fail_test(test_name, error_reason);
	}

	pass_test(test_name);
}

int main()
{
	test_small_malloc_4();
}

