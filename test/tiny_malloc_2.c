#include <stdio.h>
#include "tests.h"
#include "malloc.h"

// 3 tiny mallocs
void test_tiny_malloc_2()
{
	char *test_name = "test_tiny_malloc_2";
	char error_reason[ERROR_MSG_MAX_LEN];

	char *ptr1 = malloc(1);
	if (ptr1 == NULL)
		fail_test(test_name, "malloc(1) returned NULL");
	char *ptr2 = malloc(10);
	if (ptr2 == NULL)
		fail_test(test_name, "malloc(10) returned NULL");
	char *ptr3 = malloc(100);
	if (ptr3 == NULL)
		fail_test(test_name, "malloc(100) returned NULL");

	size_t tiny_zones_len = zone_list_len(heap_g.tiny_zones_head);
	if (tiny_zones_len != 1)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of tiny zones after 3 tiny mallocs (expected: %d, actual: %zu)\n", 1, tiny_zones_len);
		fail_test(test_name, error_reason);
	}
	size_t tiny_bin_len = free_chunk_list_len(heap_g.tiny_bin_head);
	if (tiny_bin_len != NUMBER_OF_TINY_CHUNKS_PER_ZONE - 3)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of free chunks in tiny bin after 3 tiny mallocs (expected: %zu, actual: %zu)\n", NUMBER_OF_TINY_CHUNKS_PER_ZONE - 1, tiny_bin_len);
		fail_test(test_name, error_reason);
	}

	size_t in_use_tiny_chunk_size = (size_t)(getpagesize() / 128 | IN_USE);
	size_t *chunk1_begin = (size_t *)((char *)ptr1 - SIZE_T_SIZE);
	if (*chunk1_begin != in_use_tiny_chunk_size)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected tiny chunk size for address %p (expected: %zu, actual: %zu)\n", ptr1, in_use_tiny_chunk_size, *chunk1_begin);
		fail_test(test_name, error_reason);
	}
	size_t *chunk2_begin = (size_t *)((char *)ptr2 - SIZE_T_SIZE);
	if (*chunk2_begin != in_use_tiny_chunk_size)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected tiny chunk size for address %p (expected: %zu, actual: %zu)\n", ptr2, in_use_tiny_chunk_size, *chunk2_begin);
		fail_test(test_name, error_reason);
	}
	size_t *chunk3_begin = (size_t *)((char *)ptr3 - SIZE_T_SIZE);
	if (*chunk3_begin != in_use_tiny_chunk_size)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected tiny chunk size for address %p (expected: %zu, actual: %zu)\n", ptr3, in_use_tiny_chunk_size, *chunk3_begin);
		fail_test(test_name, error_reason);
	}

	size_t *chunk1_end = (size_t *)((char *)chunk1_begin + CHUNK_SIZE_WITHOUT_FLAGS(*chunk1_begin));
	size_t *chunk2_end = (size_t *)((char *)chunk2_begin + CHUNK_SIZE_WITHOUT_FLAGS(*chunk2_begin));
	size_t *chunk3_end = (size_t *)((char *)chunk3_begin + CHUNK_SIZE_WITHOUT_FLAGS(*chunk3_begin));
	
	if (overlaps(chunk1_begin, chunk2_begin))
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "memory at [%p - %p] and [%p - %p] is overlapping)\n", chunk1_begin, chunk1_end, chunk2_begin, chunk2_end);
		fail_test(test_name, error_reason);
	}
	if (overlaps(chunk2_begin, chunk3_begin))
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "memory at [%p - %p] and [%p - %p] is overlapping)\n", chunk2_begin, chunk2_end, chunk3_begin, chunk3_end);
		fail_test(test_name, error_reason);
	}
	if (overlaps(chunk1_begin, chunk3_begin))
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "memory at [%p - %p] and [%p - %p] is overlapping)\n", chunk1_begin, chunk1_end, chunk3_begin, chunk3_end);
		fail_test(test_name, error_reason);
	}

	free(ptr1);
	free(ptr2);
	free(ptr3);

	tiny_zones_len = zone_list_len(heap_g.tiny_zones_head);
	if (tiny_zones_len != 1)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of tiny zones after 3 tiny frees (expected: %d, actual: %zu)\n", 1, tiny_zones_len);
		fail_test(test_name, error_reason);
	}
	tiny_bin_len = free_chunk_list_len(heap_g.tiny_bin_head);
	if (tiny_bin_len != NUMBER_OF_TINY_CHUNKS_PER_ZONE)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of free chunks in tiny bin after 3 tiny frees (expected: %zu, actual: %zu)\n", NUMBER_OF_TINY_CHUNKS_PER_ZONE, tiny_bin_len);
		fail_test(test_name, error_reason);
	}

	pass_test(test_name);
}

int main()
{
	test_tiny_malloc_2();
}

