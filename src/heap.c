#include <sys/mman.h>
#include "heap.h"
#include <stdlib.h>

#include <stdio.h>

// global
heap_t heap_g;

bool init_heap(void)
{
    /* Initialize tiny zone: */
    zone_header_t *initial_tiny_zone = mmap(NULL, TINY_ZONE_SIZE + ZONE_HEADER_T_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (initial_tiny_zone == MAP_FAILED)
		return false;
	//(zone_t *)(heap_g.tiny_zones_head)->remaining_free_bytes = TINY_ZONE_SIZE;
    initial_tiny_zone->prev = NULL;
    initial_tiny_zone->next = NULL;
    //(zone_t*)(heap_g.tiny_zones_head)->bin = NULL;
    heap_g.tiny_zones_head = initial_tiny_zone;

    /* Initialize tiny zone bin: */
    free_chunk_header_t *initial_tiny_free_chunk = (free_chunk_header_t *)((uint8_t *)(initial_tiny_zone) + ZONE_HEADER_T_SIZE);
    initial_tiny_free_chunk->size = TINY_ZONE_CHUNK_MAX_SIZE;
    initial_tiny_free_chunk->prev = NULL;
    initial_tiny_free_chunk->next = NULL;
    heap_g.tiny_bin_head = initial_tiny_free_chunk;

    free_chunk_header_t *next_free_chunk = NULL;
    for (size_t i = 0; i < (TINY_ZONE_SIZE / TINY_ZONE_CHUNK_MAX_SIZE); i++)
    {
        next_free_chunk = (free_chunk_header_t *)((uint8_t *)(initial_tiny_free_chunk) + initial_tiny_free_chunk->size);
        next_free_chunk->size = TINY_ZONE_CHUNK_MAX_SIZE;
        next_free_chunk->prev = initial_tiny_free_chunk;
        next_free_chunk->next = NULL;
        initial_tiny_free_chunk->next = next_free_chunk;
        initial_tiny_free_chunk = next_free_chunk;
    }

    /* Initialize small zone: */
    zone_header_t *initial_small_zone = mmap(NULL, SMALL_ZONE_SIZE + ZONE_HEADER_T_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (initial_small_zone == MAP_FAILED)
		return false;
	//(zone_t *)(heap_g.small_zones_head)->remaining_free_bytes = SMALL_ZONE_SIZE;
    initial_small_zone->prev = NULL;
    initial_small_zone->next = NULL;
    //(zone_t*)(heap_g.small_zones_head)->bin = NULL;
    heap_g.small_zones_head = initial_small_zone;

    /* Initialize small zone bin: */
    free_chunk_header_t *initial_small_free_chunk = (free_chunk_header_t *)((uint8_t *)(initial_small_zone) + ZONE_HEADER_T_SIZE);
    initial_small_free_chunk->size = SMALL_ZONE_SIZE;
    initial_small_free_chunk->prev = NULL;
    initial_small_free_chunk->next = NULL;
    heap_g.small_bin_head = initial_small_free_chunk;
    size_t *footer_chunk_size = (size_t *)((uint8_t *)initial_small_free_chunk + CHUNK_SIZE_WITHOUT_FLAGS(initial_small_free_chunk->size) - SIZE_T_SIZE);
    *footer_chunk_size = CHUNK_SIZE_WITHOUT_FLAGS(initial_small_free_chunk->size);

    /* Initialize large zone: */
	heap_g.large_zones_head = NULL;

	return true;
}

void *get_small_zone_beginning(void *chunk_ptr)
{
    for (zone_header_t *zone = heap_g.small_zones_head; zone != NULL; zone = zone->next)
    {
        if (chunk_ptr > (void *)zone && chunk_ptr < (void *)((uint8_t *)zone + SMALL_ZONE_SIZE + ZONE_HEADER_T_SIZE))
        {
            return (uint8_t *)zone + ZONE_HEADER_T_SIZE;
        }
    }
    return NULL;
}
