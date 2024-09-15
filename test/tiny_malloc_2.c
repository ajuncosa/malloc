#include "test_utils.h"
#include "malloc.h"

// 3 tiny mallocs
int main()
{
    char *ptr1 = malloc(1);
    ASSERT_POINTER_NE(ptr1, NULL);
    ASSERT_ALIGNMENT(ptr1);
    char *ptr2 = malloc(5);
    ASSERT_POINTER_NE(ptr2, NULL);
    ASSERT_ALIGNMENT(ptr2);
    char *ptr3 = malloc(TINY_ZONE_MAX_CHUNK_SIZE - SIZE_T_SIZE);
    ASSERT_POINTER_NE(ptr3, NULL);
    ASSERT_ALIGNMENT(ptr3);

    ASSERT_POINTER_NE(heap_g.tiny_zones_head, NULL);
    ASSERT_POINTER_EQ(heap_g.small_zones_head, NULL);
    ASSERT_POINTER_EQ(heap_g.large_zones_head, NULL);
    ASSERT_POINTER_NE(heap_g.tiny_bin_head, NULL);
    ASSERT_POINTER_EQ(heap_g.small_bin_head, NULL);
    ASSERT_POINTER_EQ(heap_g.small_unsorted_list_head, NULL);

    ASSERT_SIZE_EQ(zone_list_len(heap_g.tiny_zones_head), 1);
    ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.tiny_bin_head), NUMBER_OF_TINY_CHUNKS_PER_ZONE - 3);

    size_t in_use_tiny_chunk_size = (size_t)(TINY_ZONE_MAX_CHUNK_SIZE | IN_USE);

    size_t *chunk1_begin = (size_t *)((char *)ptr1 - SIZE_T_SIZE);
    ASSERT_SIZE_EQ(*chunk1_begin, in_use_tiny_chunk_size);

    size_t *chunk2_begin = (size_t *)((char *)ptr2 - SIZE_T_SIZE);
    ASSERT_SIZE_EQ(*chunk2_begin, in_use_tiny_chunk_size);

    size_t *chunk3_begin = (size_t *)((char *)ptr3 - SIZE_T_SIZE);
    ASSERT_SIZE_EQ(*chunk3_begin, in_use_tiny_chunk_size);

    ASSERT_NO_CHUNK_OVERLAP(chunk1_begin, chunk2_begin);
    ASSERT_NO_CHUNK_OVERLAP(chunk2_begin, chunk3_begin);
    ASSERT_NO_CHUNK_OVERLAP(chunk1_begin, chunk3_begin);

    free(ptr1);
    free(ptr2);
    free(ptr3);

    ASSERT_SIZE_EQ(zone_list_len(heap_g.tiny_zones_head), 1);
    ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.tiny_bin_head), NUMBER_OF_TINY_CHUNKS_PER_ZONE);
}
