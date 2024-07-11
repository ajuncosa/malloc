#include <stdio.h>
#include "tests.h"
#include "malloc.h"

// malloc(1)
void test_tiny_malloc_1()
{
	char *test_name = "test_tiny_malloc_1";
	char error_reason[ERROR_MSG_MAX_LEN];

	char *ptr = malloc(1);
	if (ptr == NULL)
		fail_test(test_name, "malloc(1) returned NULL");

	if (heap_g.tiny_zones_head == NULL)
		fail_test(test_name, "tiny zones head pointer is NULL after tiny malloc");
	if (heap_g.small_zones_head != NULL)
		fail_test(test_name, "small zones head pointer is not NULL after tiny malloc");
	if (heap_g.large_zones_head != NULL)
		fail_test(test_name, "large zones head pointer is not NULL after tiny malloc");
	if (heap_g.tiny_bin_head == NULL)
		fail_test(test_name, "tiny bin head pointer is NULL after tiny malloc");
	if (heap_g.small_bin_head != NULL)
		fail_test(test_name, "small bin head pointer is not NULL after tiny malloc");
	if (heap_g.small_unsorted_list_head != NULL)
		fail_test(test_name, "small unsorted list head pointer is not NULL after tiny malloc");

	size_t tiny_zones_len = zone_list_len(heap_g.tiny_zones_head);
	if (tiny_zones_len != 1)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of tiny zones after tiny malloc (expected: %d, actual: %zu)\n", 1, tiny_zones_len);
		fail_test(test_name, error_reason);
	}
	size_t tiny_bin_len = free_chunk_list_len(heap_g.tiny_bin_head);
	if (tiny_bin_len != NUMBER_OF_TINY_CHUNKS_PER_ZONE - 1)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of free chunks in tiny bin after tiny malloc (expected: %zu, actual: %zu)\n", NUMBER_OF_TINY_CHUNKS_PER_ZONE - 1, tiny_bin_len);
		fail_test(test_name, error_reason);
	}
	free(ptr);
	tiny_zones_len = zone_list_len(heap_g.tiny_zones_head);
	if (tiny_zones_len != 1)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of tiny zones after tiny free (expected: %d, actual: %zu)\n", 1, tiny_zones_len);
		fail_test(test_name, error_reason);
	}
	tiny_bin_len = free_chunk_list_len(heap_g.tiny_bin_head);
	if (tiny_bin_len != NUMBER_OF_TINY_CHUNKS_PER_ZONE)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of free chunks in tiny bin after tiny free (expected: %zu, actual: %zu)\n", NUMBER_OF_TINY_CHUNKS_PER_ZONE, tiny_bin_len);
		fail_test(test_name, error_reason);
	}

	pass_test(test_name);
}

int main()
{
	test_tiny_malloc_1();
}
