#include <sys/mman.h>
#include <stdio.h>
#include <stdint.h>
#include "malloc.h"
#include "heap.h"
#include "utils.h"

void *malloc(size_t size)
{
    if (size == 0)
        return NULL;

    static bool heap_initialized = false;
    if (heap_initialized == false)
    {
        if (init_heap() == false)
            return NULL;
        heap_initialized = true;
    }

    size_t aligned_chunk_size = ALIGN(size + SIZE_T_SIZE);

    /* LARGE ALLOCATION */
    if (aligned_chunk_size > heap_g.small_zone_chunk_max_size)
        return allocate_large_chunk(aligned_chunk_size);
    /* TINY ALLOCATION */
    else if (aligned_chunk_size <= heap_g.tiny_zone_chunk_max_size)
        return allocate_tiny_chunk();
    /* SMALL ALLOCATION */
    else
        return allocate_small_chunk(aligned_chunk_size);
}

void free(void *ptr)
{
    if (ptr == NULL)
        return;

    size_t *ptr_to_chunk = (size_t *)((uint8_t *)ptr - SIZE_T_SIZE);
    if ((*ptr_to_chunk & IN_USE) == 0)
        return;

    /* LARGE FREE */
    if (CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) > heap_g.small_zone_chunk_max_size)
        free_large_chunk(ptr_to_chunk);
    /* TINY FREE */
    else if (CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) == heap_g.tiny_zone_chunk_max_size)
        free_tiny_chunk(ptr_to_chunk);
    /* SMALL FREE */
    else if (CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) > heap_g.tiny_zone_chunk_max_size
        && CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) <= heap_g.small_zone_chunk_max_size)
        free_small_chunk(ptr_to_chunk);
}

void *realloc(void *ptr, size_t size)
{
    if (ptr == NULL)
        return malloc(size);
    if (size == 0)
        return NULL;

    size_t *ptr_to_chunk = (size_t *)((uint8_t *)ptr - SIZE_T_SIZE);
    if ((*ptr_to_chunk & IN_USE) == 0)
        return NULL;

    size_t aligned_new_chunk_size = ALIGN(size + SIZE_T_SIZE);
    if (aligned_new_chunk_size == CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk))
        return ptr;

    /* LARGE REALLOC */
    if (CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) > heap_g.small_zone_chunk_max_size)
        return realloc_large_chunk(ptr, ptr_to_chunk, size);
    /* TINY REALLOC */
    else if (CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) == heap_g.tiny_zone_chunk_max_size)
        return realloc_tiny_chunk(ptr, ptr_to_chunk, size, aligned_new_chunk_size);
    /* SMALL REALLOC */
    else if (CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) > heap_g.tiny_zone_chunk_max_size
         && CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) <= heap_g.small_zone_chunk_max_size)
         return realloc_small_chunk(ptr, ptr_to_chunk, size, aligned_new_chunk_size);
    
    return NULL;
}

void show_alloc_mem(void)
{
    size_t total_bytes = 0;

    for (zone_header_t *tiny_zone = get_zone_list_last(heap_g.tiny_zones_head); tiny_zone != NULL; tiny_zone = tiny_zone->prev)
    {
        print_str("TINY: ");
        print_address_hex(tiny_zone);
        print_endl();

        size_t *chunk_ptr = (size_t *)((uint8_t *)tiny_zone + ZONE_HEADER_T_SIZE);
        size_t *next_chunk = NULL;
        size_t chunk_data_size = 0;
        for (size_t i = 0; i < ((heap_g.tiny_zone_size - ZONE_HEADER_T_SIZE) / heap_g.tiny_zone_chunk_max_size); i++)
        {
            next_chunk = (size_t *)((uint8_t *)chunk_ptr + CHUNK_SIZE_WITHOUT_FLAGS(*chunk_ptr));
            if ((*chunk_ptr & IN_USE) == IN_USE)
            {
                chunk_data_size = CHUNK_SIZE_WITHOUT_FLAGS(*chunk_ptr) - SIZE_T_SIZE;
                print_chunk_info((uint8_t *)chunk_ptr + SIZE_T_SIZE, next_chunk, chunk_data_size);
                total_bytes += chunk_data_size;
            }
            chunk_ptr = next_chunk;
        }
        print_endl();
    }

    for (zone_header_t *small_zone = get_zone_list_last(heap_g.small_zones_head); small_zone != NULL; small_zone = small_zone->prev)
    {
        print_str("SMALL: ");
        print_address_hex(small_zone);
        print_endl();

        size_t *chunk_ptr = (size_t *)((uint8_t *)small_zone + ZONE_HEADER_T_SIZE);
        size_t *next_chunk = NULL;
        size_t chunk_data_size;
        size_t chunk_size;
        for (size_t i = 0; i < (heap_g.small_zone_size - ZONE_HEADER_T_SIZE); i += chunk_size)
        {
            chunk_size = CHUNK_SIZE_WITHOUT_FLAGS(*chunk_ptr);
            next_chunk = (size_t *)((uint8_t *)chunk_ptr + chunk_size);
            if ((*chunk_ptr & IN_USE) == IN_USE)
            {
                chunk_data_size = chunk_size - SIZE_T_SIZE;
                print_chunk_info((uint8_t *)chunk_ptr + SIZE_T_SIZE, next_chunk, chunk_data_size);
                total_bytes += chunk_data_size;
            }
            chunk_ptr = next_chunk;
        }
        print_endl();
    }

    for (zone_header_t *large_zone = get_zone_list_last(heap_g.large_zones_head); large_zone != NULL; large_zone = large_zone->prev)
    {
        print_str("LARGE: ");
        print_address_hex(large_zone);
        print_endl();

        size_t *chunk_ptr = (size_t *)((uint8_t *)large_zone + ZONE_HEADER_T_SIZE);
        size_t chunk_data_size = CHUNK_SIZE_WITHOUT_FLAGS(*chunk_ptr) - SIZE_T_SIZE;
        print_chunk_info((uint8_t *)chunk_ptr + SIZE_T_SIZE, (uint8_t *)chunk_ptr + CHUNK_SIZE_WITHOUT_FLAGS(*chunk_ptr), chunk_data_size);
        total_bytes += chunk_data_size;
        print_endl();
    }

    print_str("Total: ");
    print_size(total_bytes);
    print_str(" bytes\n");
}
