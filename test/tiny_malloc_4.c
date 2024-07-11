#include <stdio.h>
#include "tests.h"
#include "malloc.h"

 // NUMBER_OF_TINY_CHUNKS_PER_ZONE + 1 tiny mallocs
void test_tiny_malloc_4()
{
	char *test_name = "test_tiny_malloc_4";
	char error_reason[ERROR_MSG_MAX_LEN];
	size_t in_use_tiny_chunk_size = (size_t)(getpagesize() / 128 | IN_USE);

	size_t *prev_chunk_begin = NULL;
	char *ptr = NULL;
	for (size_t i = 0; i < NUMBER_OF_TINY_CHUNKS_PER_ZONE + 1; i++)
	{
		ptr = malloc(1);
		if (ptr == NULL)
			fail_test(test_name, "malloc(1) returned NULL");
		size_t *chunk_begin = (size_t *)((char *)ptr - SIZE_T_SIZE);
		if (*chunk_begin != in_use_tiny_chunk_size)
		{
			snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected tiny chunk size for address %p (expected: %zu, actual: %zu)\n", ptr, in_use_tiny_chunk_size, *chunk_begin);
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

	size_t tiny_zones_len = zone_list_len(heap_g.tiny_zones_head);
	if (tiny_zones_len != 2)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of tiny zones after %zu tiny mallocs (expected: %d, actual: %zu)\n", NUMBER_OF_TINY_CHUNKS_PER_ZONE + 1, 2, tiny_zones_len);
		fail_test(test_name, error_reason);
	}
	size_t tiny_bin_len = free_chunk_list_len(heap_g.tiny_bin_head);
	if (tiny_bin_len != (NUMBER_OF_TINY_CHUNKS_PER_ZONE * 2) - (NUMBER_OF_TINY_CHUNKS_PER_ZONE + 1))
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of free chunks in tiny bin after %zu tiny mallocs (expected: %zu, actual: %zu)\n", NUMBER_OF_TINY_CHUNKS_PER_ZONE + 1, (heap_g.tiny_zone_size * 2) - (NUMBER_OF_TINY_CHUNKS_PER_ZONE + 1), tiny_bin_len);
		fail_test(test_name, error_reason);
	}

	for (size_t i = 0; i < NUMBER_OF_TINY_CHUNKS_PER_ZONE + 1; i++)
	{
		free(ptr);
		ptr -= heap_g.tiny_zone_chunk_max_size;
	}

	tiny_zones_len = zone_list_len(heap_g.tiny_zones_head);
	if (tiny_zones_len != 1)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of tiny zones after %zu tiny frees (expected: %d, actual: %zu)\n", NUMBER_OF_TINY_CHUNKS_PER_ZONE + 1, 1, tiny_zones_len);
		fail_test(test_name, error_reason);
	}
	tiny_bin_len = free_chunk_list_len(heap_g.tiny_bin_head);
	if (tiny_bin_len != NUMBER_OF_TINY_CHUNKS_PER_ZONE)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of free chunks in tiny bin after %zu tiny frees (expected: %zu, actual: %zu)\n", NUMBER_OF_TINY_CHUNKS_PER_ZONE + 1, NUMBER_OF_TINY_CHUNKS_PER_ZONE, tiny_bin_len);
		fail_test(test_name, error_reason);
	}

	pass_test(test_name);
}

int main()
{
	//test_tiny_malloc_4();
}
