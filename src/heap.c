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
    free_chunk_header_t *initial_free_chunk = (free_chunk_header_t *)((uint8_t *)(initial_tiny_zone) + ZONE_HEADER_T_SIZE);
    initial_free_chunk->size = TINY_ZONE_CHUNK_MAX_SIZE;
    initial_free_chunk->prev = NULL;
    initial_free_chunk->next = NULL;
    heap_g.tiny_bin_head = initial_free_chunk;

    free_chunk_header_t *next_free_chunk = NULL;
    for (size_t i = 0; i < (TINY_ZONE_SIZE / TINY_ZONE_CHUNK_MAX_SIZE); i++)
    {
        next_free_chunk = (free_chunk_header_t *)((uint8_t *)(initial_free_chunk) + initial_free_chunk->size);
        next_free_chunk->size = TINY_ZONE_CHUNK_MAX_SIZE;
        next_free_chunk->prev = initial_free_chunk;
        next_free_chunk->next = NULL;
        initial_free_chunk->next = next_free_chunk;
        initial_free_chunk = next_free_chunk;
    }


    /* Initialize small zone: */
    //heap_g.small_zones_head = mmap(NULL, SMALL_ZONE_SIZE + ZONE_HEADER_T_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	//if (heap_g.small_zones_head == MAP_FAILED)
	//	return false;

    /* Initialize large zone: */
	heap_g.large_zones_head = NULL;

	return true;
}
