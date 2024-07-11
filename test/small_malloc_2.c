#include <stdio.h>
#include "tests.h"
#include "malloc.h"

 // 3 small mallocs
void test_small_malloc_2()
{
	char *test_name = "test_small_malloc_2";
	char error_reason[ERROR_MSG_MAX_LEN];
    size_t malloc_1_size = TINY_ZONE_MAX_CHUNK_SIZE + 1;
    size_t malloc_2_size = 10000;
    size_t malloc_3_size = SMALL_ZONE_MAX_CHUNK_SIZE - SIZE_T_SIZE;

	char *ptr1 = malloc(malloc_1_size);
	if (ptr1 == NULL)
    {
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "malloc(%zu) returned NULL\n", malloc_1_size);
		fail_test(test_name, error_reason);
    }
	char *ptr2 = malloc(malloc_2_size);
	if (ptr2 == NULL)
    {
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "malloc(%zu) returned NULL\n", malloc_2_size);
		fail_test(test_name, error_reason);
    }
    char *ptr3 = malloc(malloc_3_size);
	if (ptr3 == NULL)
    {
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "malloc(%zu) returned NULL\n", malloc_3_size);
		fail_test(test_name, error_reason);
    }

	size_t small_zones_len = zone_list_len(heap_g.small_zones_head);
	if (small_zones_len != 1)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of small zones after 3 small mallocs (expected: %d, actual: %zu)\n", 1, small_zones_len);
		fail_test(test_name, error_reason);
	}
	size_t small_bin_len = free_chunk_list_len(heap_g.small_bin_head);
	if (small_bin_len != 1)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of free chunks in small bin after 3 small mallocs (expected: %d, actual: %zu)\n", 1, small_bin_len);
		fail_test(test_name, error_reason);
	}
    size_t small_unsorted_list_len = free_chunk_list_len(heap_g.small_unsorted_list_head);
	if (small_unsorted_list_len != 0)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of free chunks in small unsorted list after 3 small mallocs (expected: %d, actual: %zu)\n", 0, small_unsorted_list_len);
		fail_test(test_name, error_reason);
	}

	size_t *chunk1_begin = (size_t *)((char *)ptr1 - SIZE_T_SIZE);
	if (*chunk1_begin != (ALIGN(malloc_1_size + SIZE_T_SIZE) | IN_USE))
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected small chunk size for address %p (expected: %zu, actual: %zu)\n", ptr1, (ALIGN(malloc_1_size + SIZE_T_SIZE) | IN_USE), *chunk1_begin);
		fail_test(test_name, error_reason);
	}
	size_t *chunk2_begin = (size_t *)((char *)ptr2 - SIZE_T_SIZE);
	if (*chunk2_begin != (ALIGN(malloc_2_size + SIZE_T_SIZE) | IN_USE))
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected small chunk size for address %p (expected: %zu, actual: %zu)\n", ptr2, (ALIGN(malloc_2_size + SIZE_T_SIZE) | IN_USE), *chunk2_begin);
		fail_test(test_name, error_reason);
	}
	size_t *chunk3_begin = (size_t *)((char *)ptr3 - SIZE_T_SIZE);
	if (*chunk3_begin != (ALIGN(malloc_3_size + SIZE_T_SIZE) | IN_USE))
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected small chunk size for address %p (expected: %zu, actual: %zu)\n", ptr3, (ALIGN(malloc_3_size + SIZE_T_SIZE) | IN_USE), *chunk3_begin);
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

	small_zones_len = zone_list_len(heap_g.small_zones_head);
	if (small_zones_len != 1)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of small zones after 3 small frees (expected: %d, actual: %zu)\n", 1, small_zones_len);
		fail_test(test_name, error_reason);
	}
	small_bin_len = free_chunk_list_len(heap_g.small_bin_head);
	if (small_bin_len != 1)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of free chunks in small bin after 3 small frees (expected: %d, actual: %zu)\n", 1, small_bin_len);
		fail_test(test_name, error_reason);
	}
    small_unsorted_list_len = free_chunk_list_len(heap_g.small_unsorted_list_head);
	if (small_unsorted_list_len != 3)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of free chunks in small unsorted list after 3 small frees (expected: %d, actual: %zu)\n", 3, small_unsorted_list_len);
		fail_test(test_name, error_reason);
	}

	pass_test(test_name);
}

int main()
{
	test_small_malloc_2();
}
