#include "test_utils.h"
#include "malloc.h"

 // 3 large mallocs
int main()
{
    size_t malloc_1_size = SMALL_ZONE_MAX_CHUNK_SIZE - SIZE_T_SIZE + 1;
    size_t malloc_2_size = 500000;
    size_t malloc_3_size = 1000000;

	char *ptr1 = malloc(malloc_1_size);
	ASSERT_POINTER_NE(ptr1, NULL);
	char *ptr2 = malloc(malloc_2_size);
	ASSERT_POINTER_NE(ptr2, NULL);
    char *ptr3 = malloc(malloc_3_size);
	ASSERT_POINTER_NE(ptr3, NULL);

	ASSERT_POINTER_EQ(heap_g.tiny_zones_head, NULL);
	ASSERT_POINTER_EQ(heap_g.small_zones_head, NULL);
	ASSERT_POINTER_NE(heap_g.large_zones_head, NULL);
	ASSERT_POINTER_EQ(heap_g.tiny_bin_head, NULL);
	ASSERT_POINTER_EQ(heap_g.small_bin_head, NULL);
	ASSERT_POINTER_EQ(heap_g.small_unsorted_list_head, NULL);

	ASSERT_SIZE_EQ(zone_list_len(heap_g.large_zones_head), 3);

    size_t *chunk1_begin = (size_t *)((char *)ptr1 - SIZE_T_SIZE);
	ASSERT_SIZE_EQ(*chunk1_begin, (ALIGN(malloc_1_size + SIZE_T_SIZE) | IN_USE));

	size_t *chunk2_begin = (size_t *)((char *)ptr2 - SIZE_T_SIZE);
	ASSERT_SIZE_EQ(*chunk2_begin, (ALIGN(malloc_2_size + SIZE_T_SIZE) | IN_USE));

	size_t *chunk3_begin = (size_t *)((char *)ptr3 - SIZE_T_SIZE);
	ASSERT_SIZE_EQ(*chunk3_begin, (ALIGN(malloc_3_size + SIZE_T_SIZE) | IN_USE));

	ASSERT_NO_CHUNK_OVERLAP(chunk1_begin, chunk2_begin);
	ASSERT_NO_CHUNK_OVERLAP(chunk2_begin, chunk3_begin);
	ASSERT_NO_CHUNK_OVERLAP(chunk1_begin, chunk3_begin);

	free(ptr1);
	free(ptr2);
	free(ptr3);

	ASSERT_SIZE_EQ(zone_list_len(heap_g.large_zones_head), 0);
}
