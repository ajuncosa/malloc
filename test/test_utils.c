#include <stdio.h>
#include <stdlib.h>
#include "tests.h"

void pass_test(char *test_name)
{
	printf(GREEN "[TEST OK] %s\n" NO_COLOR, test_name);
}

void fail_test(char *test_name, char *reason)
{
	printf(RED "[TEST KO] %s: %s\n" NO_COLOR, test_name, reason);
	exit(1);
}

size_t zone_list_len(zone_header_t *list)
{
	size_t len = 0;
	for (zone_header_t *it = list; it != NULL; it = it->next)
		len++;
	return len;
}

size_t free_chunk_list_len(free_chunk_header_t *list)
{
	size_t len = 0;
	for (free_chunk_header_t *it = list; it != NULL; it = it->next)
		len++;
	return len;
}

bool overlaps(void *ptr1, void *ptr2)
{
	size_t *chunk1_begin = (size_t *)ptr1;
	size_t *chunk2_begin = (size_t *)ptr2;
	size_t chunk1_size = CHUNK_SIZE_WITHOUT_FLAGS(*chunk1_begin);
	size_t chunk2_size = CHUNK_SIZE_WITHOUT_FLAGS(*chunk2_begin);
	size_t *chunk1_end = (size_t *)((char *)chunk1_begin + (chunk1_size));
	size_t *chunk2_end = (size_t *)((char *)chunk2_begin + chunk2_size);
	if (chunk1_begin < chunk2_begin)
	{
		if (chunk1_end > chunk2_begin)
			return true;
	}
	else if (chunk1_begin > chunk2_begin)
	{
		if (chunk2_end > chunk1_begin)
			return true;
	}
	else
		return true;

	return false;
}
