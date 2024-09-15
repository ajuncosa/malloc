#include "test_utils.h"
#include "malloc.h"

// malloc(1)
int main()
{
    char *ptr = malloc(1);
    ASSERT_POINTER_NE(ptr, NULL);
    ASSERT_ALIGNMENT(ptr);

    ASSERT_POINTER_NE(heap_g.tiny_zones_head, NULL);
    ASSERT_POINTER_EQ(heap_g.small_zones_head, NULL);
    ASSERT_POINTER_EQ(heap_g.large_zones_head, NULL);
    ASSERT_POINTER_NE(heap_g.tiny_bin_head, NULL);
    ASSERT_POINTER_EQ(heap_g.small_bin_head, NULL);
    ASSERT_POINTER_EQ(heap_g.small_unsorted_list_head, NULL);

    ASSERT_SIZE_EQ(zone_list_len(heap_g.tiny_zones_head), 1);
    ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.tiny_bin_head), NUMBER_OF_TINY_CHUNKS_PER_ZONE - 1);

    free(ptr);

    ASSERT_SIZE_EQ(zone_list_len(heap_g.tiny_zones_head), 1);
    ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.tiny_bin_head), NUMBER_OF_TINY_CHUNKS_PER_ZONE);
}
