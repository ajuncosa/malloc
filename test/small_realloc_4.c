#include "test_utils.h"
#include "malloc.h"

// small to smaller small (no split)
int main()
{
    size_t malloc_size = SMALL_ZONE_MAX_CHUNK_SIZE - SIZE_T_SIZE;
    size_t realloc_size = malloc_size - TINY_ZONE_MAX_CHUNK_SIZE;

    char *ptr = malloc(malloc_size);
    ASSERT_POINTER_NE(ptr, NULL);
    ASSERT_ALIGNMENT(ptr);
    for(size_t i = 0; i < malloc_size; i++)
        ptr[i] = 'a' + i % 26;

    ASSERT_SIZE_EQ(zone_list_len(heap_g.small_zones_head), 1);
    ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_bin_head), 1);
    ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_unsorted_list_head), 0);
    
    char *new_ptr = realloc(ptr, realloc_size);
    ASSERT_POINTER_NE(new_ptr, NULL);
    ASSERT_POINTER_EQ(new_ptr, ptr);
    ASSERT_ALIGNMENT(new_ptr);
    for(size_t i = 0; i < realloc_size; i++)
        ASSERT_CHAR_EQ(new_ptr[i], 'a' + i % 26);

    ASSERT_SIZE_EQ(zone_list_len(heap_g.small_zones_head), 1);
    ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_bin_head), 1);
    ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_unsorted_list_head), 0);

    size_t *new_chunk_begin = (size_t *)((char *)new_ptr - SIZE_T_SIZE);
    ASSERT_SIZE_EQ(*new_chunk_begin, (ALIGN(malloc_size + SIZE_T_SIZE) | IN_USE));

    free(new_ptr);

    ASSERT_SIZE_EQ(zone_list_len(heap_g.small_zones_head), 1);
    ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_bin_head), 1);
    ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_unsorted_list_head), 1);
}
