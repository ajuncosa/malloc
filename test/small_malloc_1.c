#include <stdio.h>
#include "tests.h"
#include "malloc.h"

 // malloc(129)
void test_small_malloc_1()
{
	char *test_name = "test_small_malloc_1";
	char error_reason[ERROR_MSG_MAX_LEN];

	char *ptr = malloc(129);
	if (ptr == NULL)
		fail_test(test_name, "malloc(129) returned NULL");

	if (heap_g.small_zones_head == NULL)
		fail_test(test_name, "small zones head pointer is NULL after small malloc");
	if (heap_g.small_zones_head == NULL)
		fail_test(test_name, "small zones head pointer is NULL after small malloc");
	if (heap_g.large_zones_head != NULL)
		fail_test(test_name, "large zones head pointer is not NULL after small malloc");
	if (heap_g.small_bin_head == NULL)
		fail_test(test_name, "small bin head pointer is NULL after small malloc");
	if (heap_g.small_bin_head == NULL)
		fail_test(test_name, "small bin head pointer is NULL after small malloc");
	if (heap_g.small_unsorted_list_head != NULL)
		fail_test(test_name, "small unsorted list head pointer is not NULL after small malloc");

	size_t small_zones_len = zone_list_len(heap_g.small_zones_head);
	if (small_zones_len != 1)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of small zones after small malloc (expected: %d, actual: %zu)\n", 1, small_zones_len);
		fail_test(test_name, error_reason);
	}
	size_t small_bin_len = free_chunk_list_len(heap_g.small_bin_head);
	if (small_bin_len != 1)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of free chunks in small bin after small malloc (expected: %d, actual: %zu)\n", 1, small_bin_len);
		fail_test(test_name, error_reason);
	}
    size_t small_unsorted_list_len = free_chunk_list_len(heap_g.small_unsorted_list_head);
	if (small_unsorted_list_len != 0)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of free chunks in small unsorted list after small free (expected: %d, actual: %zu)\n", 0, small_unsorted_list_len);
		fail_test(test_name, error_reason);
	}

	free(ptr);

	small_zones_len = zone_list_len(heap_g.small_zones_head);
	if (small_zones_len != 1)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of small zones after small free (expected: %d, actual: %zu)\n", 1, small_zones_len);
		fail_test(test_name, error_reason);
	}
	small_bin_len = free_chunk_list_len(heap_g.small_bin_head);
	if (small_bin_len != 1)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of free chunks in small bin after small free (expected: %d, actual: %zu)\n", 1, small_bin_len);
		fail_test(test_name, error_reason);
	}
    small_unsorted_list_len = free_chunk_list_len(heap_g.small_unsorted_list_head);
	if (small_unsorted_list_len != 1)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of free chunks in small unsorted list after small free (expected: %d, actual: %zu)\n", 1, small_unsorted_list_len);
		fail_test(test_name, error_reason);
	}

	pass_test(test_name);
}

int main()
{
	test_small_malloc_1();
}
