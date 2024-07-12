#include <stdio.h>
#include "tests.h"
#include "malloc.h"

 // malloc(SMALL_ZONE_MAX_CHUNK_SIZE - SIZE_T_SIZE + 1)
int main()
{
	char *test_name = "test_large_malloc_1";
	char error_reason[ERROR_MSG_MAX_LEN];

	char *ptr = malloc(SMALL_ZONE_MAX_CHUNK_SIZE - SIZE_T_SIZE + 1);
	if (ptr == NULL)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "malloc(%zu) returned NULL\n", SMALL_ZONE_MAX_CHUNK_SIZE + 1 - SIZE_T_SIZE);
		fail_test(test_name, error_reason);
    }

	if (heap_g.tiny_zones_head != NULL)
		fail_test(test_name, "tiny zones head pointer is not NULL after large malloc");
	if (heap_g.small_zones_head != NULL)
		fail_test(test_name, "small zones head pointer is not NULL after large malloc");
	if (heap_g.large_zones_head == NULL)
		fail_test(test_name, "large zones head pointer is NULL after large malloc");
	if (heap_g.tiny_bin_head != NULL)
		fail_test(test_name, "tiny bin head pointer is not NULL after large malloc");
	if (heap_g.small_bin_head != NULL)
		fail_test(test_name, "small bin head pointer is not NULL after large malloc");
	if (heap_g.small_unsorted_list_head != NULL)
		fail_test(test_name, "small unsorted list head pointer is not NULL after large malloc");

	size_t large_zones_len = zone_list_len(heap_g.large_zones_head);
	if (large_zones_len != 1)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of large zones after large malloc (expected: %d, actual: %zu)\n", 1, large_zones_len);
		fail_test(test_name, error_reason);
	}

	free(ptr);

	large_zones_len = zone_list_len(heap_g.large_zones_head);
	if (large_zones_len != 0)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "unexpected number of large zones after large free (expected: %d, actual: %zu)\n", 0, large_zones_len);
		fail_test(test_name, error_reason);
	}

	pass_test(test_name);
}
