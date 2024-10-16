#include "test_utils.h"
#include "malloc.h"

int main()
{
    ASSERT_SIZE_EQ(heap_g.tiny_zone_size, 0);
    ASSERT_SIZE_EQ(heap_g.small_zone_size, 0);
    ASSERT_SIZE_EQ(heap_g.tiny_zone_chunk_max_size, 0);
    ASSERT_SIZE_EQ(heap_g.small_zone_chunk_max_size, 0);
    ASSERT_POINTER_EQ(heap_g.tiny_zones_head, NULL);
    ASSERT_POINTER_EQ(heap_g.small_zones_head, NULL);
    ASSERT_POINTER_EQ(heap_g.large_zones_head, NULL);
    ASSERT_POINTER_EQ(heap_g.tiny_bin_head, NULL);
    ASSERT_POINTER_EQ(heap_g.small_bin_head, NULL);
    ASSERT_POINTER_EQ(heap_g.small_unsorted_list_head, NULL);

    char *ptr = malloc(1);
    ASSERT_SIZE_EQ(heap_g.tiny_zone_size % PAGE_SIZE, 0);
    ASSERT_SIZE_EQ(heap_g.small_zone_size % PAGE_SIZE, 0);

    free(ptr);
}
