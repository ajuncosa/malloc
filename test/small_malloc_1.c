#include "test_utils.h"
#include "malloc.h"

 // malloc(129)
int main()
{
	char *test_name = "test_small_malloc_1";

	char *ptr = malloc(TINY_ZONE_MAX_CHUNK_SIZE - SIZE_T_SIZE + 1);
	ASSERT_POINTER_NE(ptr, NULL);
	
	ASSERT_POINTER_EQ(heap_g.tiny_zones_head, NULL);
	ASSERT_POINTER_NE(heap_g.small_zones_head, NULL);
	ASSERT_POINTER_EQ(heap_g.large_zones_head, NULL);
	ASSERT_POINTER_EQ(heap_g.tiny_bin_head, NULL);
	ASSERT_POINTER_NE(heap_g.small_bin_head, NULL);
	ASSERT_POINTER_EQ(heap_g.small_unsorted_list_head, NULL);

	ASSERT_SIZE_EQ(zone_list_len(heap_g.small_zones_head), 1);
	ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_bin_head), 1);
	ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_unsorted_list_head), 0);

	free(ptr);

	ASSERT_SIZE_EQ(zone_list_len(heap_g.small_zones_head), 1);
	ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_bin_head), 1);
	ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_unsorted_list_head), 1);

	pass_test(test_name);
}
