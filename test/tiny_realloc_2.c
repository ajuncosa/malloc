#include "test_utils.h"
#include "malloc.h"

// tiny to small
int main()
{
    size_t malloc_size = TINY_ZONE_MAX_CHUNK_SIZE - SIZE_T_SIZE;
    size_t realloc_size = (TINY_ZONE_MAX_CHUNK_SIZE - SIZE_T_SIZE) * 2;

    char *ptr = malloc(malloc_size);
    ASSERT_POINTER_NE(ptr, NULL);
    ASSERT_ALIGNMENT(ptr);
    for(size_t i = 0; i < malloc_size; i++)
        ptr[i] = 'a' + i % 26;

    ASSERT_SIZE_EQ(zone_list_len(heap_g.tiny_zones_head), 1);
    ASSERT_SIZE_EQ(zone_list_len(heap_g.small_zones_head), 0);
    ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.tiny_bin_head), NUMBER_OF_TINY_CHUNKS_PER_ZONE - 1);
    
    char *new_ptr = realloc(ptr, realloc_size);
    ASSERT_POINTER_NE(new_ptr, NULL);
    ASSERT_POINTER_NE(new_ptr, ptr);
    ASSERT_ALIGNMENT(new_ptr);

    ASSERT_SIZE_EQ(zone_list_len(heap_g.tiny_zones_head), 1);
    ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.tiny_bin_head), NUMBER_OF_TINY_CHUNKS_PER_ZONE);
    ASSERT_SIZE_EQ(zone_list_len(heap_g.small_zones_head), 1);
    ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_bin_head), 1);
    ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_unsorted_list_head), 0);

    for(size_t i = 0; i < malloc_size; i++)
        ASSERT_CHAR_EQ(new_ptr[i], 'a' + i % 26);

    free(new_ptr);

    ASSERT_SIZE_EQ(zone_list_len(heap_g.tiny_zones_head), 1);
    ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.tiny_bin_head), NUMBER_OF_TINY_CHUNKS_PER_ZONE);
    ASSERT_SIZE_EQ(zone_list_len(heap_g.small_zones_head), 1);
    ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_bin_head), 1);
    ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_unsorted_list_head), 1);
}
