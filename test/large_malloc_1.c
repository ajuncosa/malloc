#include "test_utils.h"
#include "malloc.h"

 // malloc(SMALL_ZONE_MAX_CHUNK_SIZE - SIZE_T_SIZE + 1)
int main()
{
	char *test_name = "test_large_malloc_1";

	char *ptr = malloc(SMALL_ZONE_MAX_CHUNK_SIZE - SIZE_T_SIZE + 1);
	ASSERT_POINTER_NE(ptr, NULL);

	ASSERT_POINTER_EQ(heap_g.tiny_zones_head, NULL);
	ASSERT_POINTER_EQ(heap_g.small_zones_head, NULL);
	ASSERT_POINTER_NE(heap_g.large_zones_head, NULL);
	ASSERT_POINTER_EQ(heap_g.tiny_bin_head, NULL);
	ASSERT_POINTER_EQ(heap_g.small_bin_head, NULL);
	ASSERT_POINTER_EQ(heap_g.small_unsorted_list_head, NULL);

	ASSERT_SIZE_EQ(zone_list_len(heap_g.large_zones_head), 1);

	free(ptr);

	ASSERT_SIZE_EQ(zone_list_len(heap_g.large_zones_head), 0);

	pass_test(test_name);
}
