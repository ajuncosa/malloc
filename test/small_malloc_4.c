#include "test_utils.h"
#include "malloc.h"

// MIN_NUMBER_OF_SMALL_CHUNKS_PER_ZONE + 1 small mallocs
int main()
{
    size_t malloc_size = SMALL_ZONE_MAX_CHUNK_SIZE - SIZE_T_SIZE;

    size_t *prev_chunk_begin = NULL;
    char *ptr = NULL;
    for (size_t i = 0; i < MIN_NUMBER_OF_SMALL_CHUNKS_PER_ZONE; i++)
    {
        ptr = malloc(malloc_size);
        ASSERT_POINTER_NE(ptr, NULL);
        ASSERT_ALIGNMENT(ptr);

        size_t *chunk_begin = (size_t *)((char *)ptr - SIZE_T_SIZE);
        ASSERT_SIZE_EQ(*chunk_begin, (ALIGN(malloc_size + SIZE_T_SIZE) | IN_USE));
        ASSERT_NO_CHUNK_OVERLAP(chunk_begin, prev_chunk_begin);
        prev_chunk_begin = chunk_begin;
    }
    char *ptr_last = malloc(malloc_size); // extra one (in a separate zone)
    ASSERT_POINTER_NE(ptr_last, NULL);
    ASSERT_ALIGNMENT(ptr_last);

    size_t *chunk_begin = (size_t *)((char *)ptr_last - SIZE_T_SIZE);
    ASSERT_SIZE_EQ(*chunk_begin, (ALIGN(malloc_size + SIZE_T_SIZE) | IN_USE));
    ASSERT_NO_CHUNK_OVERLAP(chunk_begin, prev_chunk_begin);

    ASSERT_POINTER_EQ(heap_g.tiny_zones_head, NULL);
    ASSERT_POINTER_NE(heap_g.small_zones_head, NULL);
    ASSERT_POINTER_EQ(heap_g.large_zones_head, NULL);
    ASSERT_POINTER_EQ(heap_g.tiny_bin_head, NULL);
    ASSERT_POINTER_NE(heap_g.small_bin_head, NULL);
    ASSERT_POINTER_EQ(heap_g.small_unsorted_list_head, NULL);

    ASSERT_SIZE_EQ(zone_list_len(heap_g.small_zones_head), 2);
    ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_bin_head), 2);
    ASSERT_SIZE_EQ(CHUNK_SIZE_WITHOUT_FLAGS(heap_g.small_bin_head->size), heap_g.small_zone_size - ZONE_HEADER_T_SIZE - (MIN_NUMBER_OF_SMALL_CHUNKS_PER_ZONE * (ALIGN(malloc_size + SIZE_T_SIZE))));
    ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_unsorted_list_head), 0);

    for (size_t i = 0; i < MIN_NUMBER_OF_SMALL_CHUNKS_PER_ZONE; i++)
    {
        free(ptr);
        size_t *chunk_begin = (size_t *)((char *)ptr - SIZE_T_SIZE);
        ASSERT_SIZE_EQ(*chunk_begin, (ALIGN(malloc_size + SIZE_T_SIZE)));
        if (i > 0)
        {
            size_t *next_chunk_begin = (size_t *)((char *)chunk_begin + CHUNK_SIZE_WITHOUT_FLAGS(*chunk_begin));
            ASSERT_SIZE_EQ(*next_chunk_begin, (ALIGN(malloc_size + SIZE_T_SIZE) | PREVIOUS_FREE));
        }
        ptr -= heap_g.small_zone_chunk_max_size;
    }
    free(ptr_last);

    ASSERT_SIZE_EQ(zone_list_len(heap_g.small_zones_head), 2);
    ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_bin_head), 2);
    ASSERT_SIZE_EQ(CHUNK_SIZE_WITHOUT_FLAGS(heap_g.small_bin_head->size), heap_g.small_zone_size - ZONE_HEADER_T_SIZE - (MIN_NUMBER_OF_SMALL_CHUNKS_PER_ZONE * (ALIGN(malloc_size + SIZE_T_SIZE))));
    ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_unsorted_list_head), MIN_NUMBER_OF_SMALL_CHUNKS_PER_ZONE + 1);

    ptr = malloc(malloc_size);
    ASSERT_POINTER_NE(ptr, NULL);
    ASSERT_SIZE_EQ(zone_list_len(heap_g.small_zones_head), 2);

    free(ptr);

    ptr = malloc(malloc_size - 100);
    ASSERT_POINTER_NE(ptr, NULL);

    ASSERT_SIZE_EQ(zone_list_len(heap_g.small_zones_head), 1);
    ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_bin_head), 1);
    ASSERT_SIZE_EQ(CHUNK_SIZE_WITHOUT_FLAGS(heap_g.small_bin_head->size), heap_g.small_zone_size - ZONE_HEADER_T_SIZE - (ALIGN(malloc_size - 100 + SIZE_T_SIZE)));
    ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_unsorted_list_head), 0);

    free(ptr);

    ASSERT_SIZE_EQ(zone_list_len(heap_g.small_zones_head), 1);
    ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_bin_head), 1);
    ASSERT_SIZE_EQ(CHUNK_SIZE_WITHOUT_FLAGS(heap_g.small_bin_head->size), heap_g.small_zone_size - ZONE_HEADER_T_SIZE - (ALIGN(malloc_size - 100 + SIZE_T_SIZE)));
    ASSERT_SIZE_EQ(free_chunk_list_len(heap_g.small_unsorted_list_head), 1);
}
