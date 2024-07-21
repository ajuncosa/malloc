#include <stdio.h>
#include "tests.h"
#include "malloc.h"

// tiny to tiny
int main()
{
	char *test_name = "test_tiny_realloc_1";

	char *ptr = malloc(50);
	ASSERT_POINTER_NE(ptr, NULL);
	for(size_t i = 0; i < 50; i++)
		ptr[i] = 'a' + i % 26;

	ASSERT_SIZE_EQ(zone_list_len(heap_g.tiny_zones_head), 1);
	ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.tiny_bin_head), NUMBER_OF_TINY_CHUNKS_PER_ZONE - 1);
    
    char *new_ptr = realloc(ptr, 10);
	ASSERT_POINTER_NE(new_ptr, NULL);
	ASSERT_POINTER_EQ(new_ptr, ptr);
	for(size_t i = 0; i < 10; i++)
		ASSERT_CHAR_EQ(new_ptr[i], 'a' + i % 26);

	free(new_ptr);

	ASSERT_SIZE_EQ(zone_list_len(heap_g.tiny_zones_head), 1);
	ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.tiny_bin_head), NUMBER_OF_TINY_CHUNKS_PER_ZONE);

	pass_test(test_name);
}
