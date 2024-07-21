#include "test_utils.h"
#include "malloc.h"

// small to large
int main()
{
	char *test_name = "test_small_realloc_2";
    size_t malloc_size = TINY_ZONE_MAX_CHUNK_SIZE - SIZE_T_SIZE + 1;

	char *ptr = malloc(malloc_size);
	ASSERT_POINTER_NE(ptr, NULL);
	for(size_t i = 0; i < malloc_size; i++)
		ptr[i] = 'a' + i % 26;

	ASSERT_SIZE_EQ(zone_list_len(heap_g.small_zones_head), 1);
	ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_bin_head), 1);
	ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_unsorted_list_head), 0);
	ASSERT_SIZE_EQ(zone_list_len(heap_g.large_zones_head), 0);
    
    char *new_ptr = realloc(ptr, SMALL_ZONE_MAX_CHUNK_SIZE - SIZE_T_SIZE + 1);
	ASSERT_POINTER_NE(new_ptr, NULL);
	ASSERT_POINTER_NE(new_ptr, ptr);
	for(size_t i = 0; i < malloc_size; i++)
		ASSERT_CHAR_EQ(new_ptr[i], 'a' + i % 26);

	ASSERT_SIZE_EQ(zone_list_len(heap_g.small_zones_head), 1);
	ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_bin_head), 1);
	ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_unsorted_list_head), 1);
	ASSERT_SIZE_EQ(zone_list_len(heap_g.large_zones_head), 1);

	free(new_ptr);

	ASSERT_SIZE_EQ(zone_list_len(heap_g.small_zones_head), 1);
	ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_bin_head), 1);
	ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_unsorted_list_head), 1);
	ASSERT_SIZE_EQ(zone_list_len(heap_g.large_zones_head), 0);

	pass_test(test_name);
}
