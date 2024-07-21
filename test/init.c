#include <stdio.h>
#include "test_utils.h"
#include "malloc.h"

int main()
{
	char *test_name = "test_init";
	char error_reason[ERROR_MSG_MAX_LEN];

	if (heap_g.tiny_zone_size != 0)
		fail_test(test_name, "tiny zone size is not properly initialized to 0");
	if (heap_g.small_zone_size != 0)
		fail_test(test_name, "small zone size is not properly initialized to 0");
	if (heap_g.tiny_zone_chunk_max_size != 0)
		fail_test(test_name, "tiny zone chunk maximum size is not properly initialized to 0");
	if (heap_g.small_zone_chunk_max_size != 0)
		fail_test(test_name, "small zone chunk maximum size is not properly initialized to 0");
	if (heap_g.tiny_zones_head != NULL)
		fail_test(test_name, "tiny zones head pointer is not properly initialized to NULL");
	if (heap_g.small_zones_head != NULL)
		fail_test(test_name, "small zones head pointer is not properly initialized to NULL");
	if (heap_g.large_zones_head != NULL)
		fail_test(test_name, "large zones head pointer is not properly initialized to NULL");
	if (heap_g.tiny_bin_head != NULL)
		fail_test(test_name, "tiny bin head pointer is not properly initialized to NULL");
	if (heap_g.small_bin_head != NULL)
		fail_test(test_name, "small bin head pointer is not properly initialized to NULL");
	if (heap_g.small_unsorted_list_head != NULL)
		fail_test(test_name, "small unsorted list head pointer is not properly initialized to NULL");

	char *ptr = malloc(1);
	if (heap_g.tiny_zone_size % getpagesize() != 0)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "tiny zone size (%u) is not a multiple of getpagesize()\n", heap_g.tiny_zone_size);
		fail_test(test_name, error_reason);
	}
	if (heap_g.small_zone_size % getpagesize() != 0)
	{
		snprintf(error_reason, ERROR_MSG_MAX_LEN, "tiny zone size (%u) is not a multiple of getpagesize()\n", heap_g.tiny_zone_size);
		fail_test(test_name, error_reason);
	}
	free(ptr);

	pass_test(test_name);
}
