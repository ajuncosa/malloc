#include <sys/mman.h>
#include <stdlib.h>
#include <errno.h>
#include "heap.h"

#include <stdio.h>

// global
heap_t heap_g;

bool init_heap(void)
{
    /* Initialize tiny zone: */
    heap_g.tiny_zones_head = NULL;
    if (allocate_new_tiny_zone() == NULL)
        return false;

    /* Initialize small zone: */
    heap_g.small_zones_head = NULL;
    if (allocate_new_small_zone() == NULL)
        return false;

    /* Initialize large zone: */
	heap_g.large_zones_head = NULL;

	return true;
}

zone_header_t *allocate_new_tiny_zone()
{
    printf ("ALLOCATING NEW TINY ZONE\n");
    zone_header_t *new_tiny_zone = mmap(NULL, TINY_ZONE_SIZE + ZONE_HEADER_T_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (new_tiny_zone == MAP_FAILED)
		return NULL;

	//(zone_t *)(heap_g.tiny_zones_head)->remaining_free_bytes = TINY_ZONE_SIZE;
    new_tiny_zone->prev = NULL;
    new_tiny_zone->next = heap_g.tiny_zones_head;
    if (heap_g.tiny_zones_head != NULL)
    {
        zone_header_t *tiny_head = (zone_header_t*)heap_g.tiny_zones_head;
        tiny_head->prev = new_tiny_zone;
    }
    //(zone_t*)(heap_g.tiny_zones_head)->bin = NULL;
    heap_g.tiny_zones_head = new_tiny_zone;

    /* Initialize tiny zone bin: */
    free_chunk_header_t *initial_tiny_free_chunk = (free_chunk_header_t *)((uint8_t *)(new_tiny_zone) + ZONE_HEADER_T_SIZE);
    initial_tiny_free_chunk->size = TINY_ZONE_CHUNK_MAX_SIZE;
    initial_tiny_free_chunk->prev = NULL;
    initial_tiny_free_chunk->next = NULL;
    heap_g.tiny_bin_head = initial_tiny_free_chunk;

    free_chunk_header_t *next_free_chunk = NULL;
    for (size_t i = 0; i < (TINY_ZONE_SIZE / TINY_ZONE_CHUNK_MAX_SIZE) - 1; i++)
    {
        next_free_chunk = (free_chunk_header_t *)((uint8_t *)(initial_tiny_free_chunk) + initial_tiny_free_chunk->size);
        next_free_chunk->size = TINY_ZONE_CHUNK_MAX_SIZE;
        next_free_chunk->prev = initial_tiny_free_chunk;
        next_free_chunk->next = NULL;
        initial_tiny_free_chunk->next = next_free_chunk;
        initial_tiny_free_chunk = next_free_chunk;
    }

    return new_tiny_zone;
}

zone_header_t *allocate_new_small_zone()
{
    zone_header_t *new_small_zone = mmap(NULL, SMALL_ZONE_SIZE + ZONE_HEADER_T_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (new_small_zone == MAP_FAILED)
		return NULL;

	//(zone_t *)(heap_g.small_zones_head)->remaining_free_bytes = SMALL_ZONE_SIZE;
    new_small_zone->prev = NULL;
    new_small_zone->next = heap_g.small_zones_head;
    if (heap_g.small_zones_head != NULL)
    {
        zone_header_t *small_head = (zone_header_t*)heap_g.small_zones_head;
        small_head->prev = new_small_zone;
    }
    //(zone_t*)(heap_g.small_zones_head)->bin = NULL;
    heap_g.small_zones_head = new_small_zone;

    /* Initialize small zone bin: */
    free_chunk_header_t *initial_small_free_chunk = (free_chunk_header_t *)((uint8_t *)(new_small_zone) + ZONE_HEADER_T_SIZE);
    initial_small_free_chunk->size = SMALL_ZONE_SIZE;
    initial_small_free_chunk->prev = NULL;
    initial_small_free_chunk->next = NULL;
    heap_g.small_bin_head = initial_small_free_chunk;
    size_t *footer_chunk_size = (size_t *)((uint8_t *)initial_small_free_chunk + CHUNK_SIZE_WITHOUT_FLAGS(initial_small_free_chunk->size) - SIZE_T_SIZE);
    *footer_chunk_size = CHUNK_SIZE_WITHOUT_FLAGS(initial_small_free_chunk->size);

    return new_small_zone;
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

void *allocate_large_chunk(size_t chunk_size)
{
	zone_header_t *new_zone = mmap(NULL, chunk_size + ZONE_HEADER_T_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (new_zone == MAP_FAILED)
		return NULL;

	new_zone->next = NULL;
	new_zone->prev = NULL;

	size_t *new_chunk = (size_t *)((uint8_t *)new_zone + ZONE_HEADER_T_SIZE);
	*new_chunk = chunk_size | IN_USE;

	// insert new allocated chunk at the front of the large list:
	if (heap_g.large_zones_head != NULL)
	{
		new_zone->next = heap_g.large_zones_head;
		zone_header_t *large_head = (zone_header_t *)(heap_g.large_zones_head);
		large_head->prev = new_zone;
	}
	heap_g.large_zones_head = new_zone;

	return (uint8_t *)new_chunk + SIZE_T_SIZE;
}

void *allocate_small_chunk(size_t chunk_size)
{
	free_chunk_header_t *new_chunk = NULL;
	// TODO: improve this search for a better match
	for (free_chunk_header_t *chunk = heap_g.small_bin_head; chunk != NULL; chunk = chunk->next)
	{
		if (chunk->size >= chunk_size)
		{
			new_chunk = chunk;
			break;
		}
	}
	if (new_chunk ==  NULL)
	{
		// the zone is full or there is not a big enough free chunk
		printf("small zone is full\n");
		if (allocate_new_small_zone() == NULL)
        	return NULL;
		new_chunk = heap_g.small_bin_head;
	}
	printf("allocating small chunk of size %zu.\n", chunk_size);

	// If the remainder of splitting would be big enough to store
	// more small chunks, split the chunk:
	if ((CHUNK_SIZE_WITHOUT_FLAGS(new_chunk->size) - chunk_size) > TINY_ZONE_CHUNK_MAX_SIZE)
	{
		printf("Splitting chunk.\n");
		free_chunk_header_t *remaining_chunk = (free_chunk_header_t *)((uint8_t *)new_chunk + chunk_size);
		if (heap_g.small_bin_head == new_chunk)
			heap_g.small_bin_head = remaining_chunk;
		if (new_chunk->next != NULL)
			new_chunk->next->prev = remaining_chunk;
		if (new_chunk->prev != NULL)
			new_chunk->prev->next = remaining_chunk;
		remaining_chunk->size = CHUNK_SIZE_WITHOUT_FLAGS(new_chunk->size) - chunk_size;
		remaining_chunk->prev = new_chunk->prev;
		remaining_chunk->next = new_chunk->next;
		new_chunk->size = chunk_size;

		size_t *remaining_chunk_footer_size = (size_t *)((uint8_t *)remaining_chunk + remaining_chunk->size - SIZE_T_SIZE);
    	*remaining_chunk_footer_size = remaining_chunk->size;
		printf("Remaining free chunk size: %zu.\n", remaining_chunk->size);

		// not needed if in use chunks dont have valid footer:
		//size_t *new_chunk_footer_size = (size_t *)((uint8_t *)new_chunk + CHUNK_SIZE_WITHOUT_FLAGS(new_chunk->size) - SIZE_T_SIZE);
    	//*new_chunk_footer_size = CHUNK_SIZE_WITHOUT_FLAGS(new_chunk->size);
	}
	else
	{
		if (heap_g.small_bin_head == new_chunk)
			heap_g.small_bin_head = new_chunk->next;
		if (new_chunk->next != NULL)
			new_chunk->next->prev = new_chunk->prev;
		if (new_chunk->prev != NULL)
			new_chunk->prev->next = new_chunk->next;

		void *new_chunk_zone = get_small_zone_beginning(new_chunk);
		size_t *next_chunk = (size_t *)((uint8_t *)new_chunk + CHUNK_SIZE_WITHOUT_FLAGS(new_chunk->size));
		if (next_chunk != (new_chunk_zone + SMALL_ZONE_SIZE))
		{
			printf("Unsetting next chunk's PREVIOUS_FREE\n");
			*next_chunk &= ~PREVIOUS_FREE;
		}
		else
		{
			printf("NOT unsetting next chunk's PREVIOUS_FREE bc next_chunk is end of zone\n");
		}
	}

	new_chunk->prev = NULL;
	new_chunk->next = NULL;
	new_chunk->size |= IN_USE;

	return (uint8_t *)new_chunk + SIZE_T_SIZE;
}

void *allocate_tiny_chunk(void)
{
	free_chunk_header_t *new_chunk = heap_g.tiny_bin_head;
	if (new_chunk == NULL)
	{
		printf("tiny zone is full\n");
		if (allocate_new_tiny_zone() == NULL)
        	return NULL;
		new_chunk = heap_g.tiny_bin_head;
	}
	heap_g.tiny_bin_head = new_chunk->next;
	if (new_chunk->next != NULL)
		new_chunk->next->prev = NULL;
	new_chunk->prev = NULL;
	new_chunk->next = NULL;
	new_chunk->size |= IN_USE;
	return (uint8_t *)new_chunk + SIZE_T_SIZE;
}

void free_large_chunk(size_t *ptr_to_chunk)
{
	zone_header_t *ptr_to_zone = (zone_header_t *)((uint8_t *)ptr_to_chunk - ZONE_HEADER_T_SIZE);
	//printf("ptr to zone: %p, next: %p, prev: %p\n", ptr_to_zone, ptr_to_zone->next, ptr_to_zone->prev);
	if (ptr_to_zone == heap_g.large_zones_head)
		heap_g.large_zones_head = ptr_to_zone->next;
	if (ptr_to_zone->prev != NULL)
		ptr_to_zone->prev->next = ptr_to_zone->next;
	if (ptr_to_zone->next != NULL)
		ptr_to_zone->next->prev = ptr_to_zone->prev;
	if (munmap(ptr_to_zone, ALIGN(*ptr_to_chunk) + ZONE_HEADER_T_SIZE) == -1)
	{
		printf("Error: munmap failed with errno: %d\n", errno);
		exit(1);
	}
}

void free_small_chunk(size_t *ptr_to_chunk)
{
	free_chunk_header_t *freed_chunk = (free_chunk_header_t *)ptr_to_chunk;
	void *freed_chunk_zone = get_small_zone_beginning(freed_chunk);
	size_t *next_chunk = (size_t *)((uint8_t *)freed_chunk + CHUNK_SIZE_WITHOUT_FLAGS(freed_chunk->size));
	if (next_chunk != (freed_chunk_zone + SMALL_ZONE_SIZE))
	{
		printf("Setting next chunk to PREVIOUS_FREE.\n");
		*next_chunk |= PREVIOUS_FREE;
	}
	else
	{
		printf("NOT setting next chunk's PREVIOUS_FREE bc next_chunk is end of zone\n");
	}
	// If PREVIOUS_FREE is not set, the previous chunk footer size_t is not usable (those bytes might be in use by the user payload)
	if (freed_chunk != freed_chunk_zone && ((freed_chunk->size & PREVIOUS_FREE) == PREVIOUS_FREE))
	{
		printf("About to coalesce...\n");
		size_t prev_size = *(size_t *)((uint8_t *)freed_chunk - SIZE_T_SIZE);
		size_t *prev_chunk_ptr = (size_t *)((uint8_t *)freed_chunk - prev_size);
		if ((*prev_chunk_ptr & IN_USE) == 0)
		{
			printf("Colaescing with prev chunk of size: %zu.\n", prev_size);
			free_chunk_header_t *prev_free_chunk = (free_chunk_header_t *)prev_chunk_ptr;
			prev_free_chunk->size = prev_size + CHUNK_SIZE_WITHOUT_FLAGS(freed_chunk->size);

			size_t *footer_size = (size_t *)((uint8_t *)prev_free_chunk + CHUNK_SIZE_WITHOUT_FLAGS(prev_free_chunk->size) - SIZE_T_SIZE);
			*footer_size = CHUNK_SIZE_WITHOUT_FLAGS(prev_free_chunk->size);

			return;
		}
	}
	else
	{
		printf("NOT coalescing because freed chunk is beginning of zone (or the previous chunk is not free to coalesce)\n");
	}

	// TODO: sort by size?
	heap_g.small_bin_head->prev = freed_chunk;
	freed_chunk->next = heap_g.small_bin_head;
	freed_chunk->prev = NULL;
	freed_chunk->size &= ~IN_USE;
	heap_g.small_bin_head = freed_chunk;
	size_t *footer_size = (size_t *)((uint8_t *)freed_chunk + CHUNK_SIZE_WITHOUT_FLAGS(freed_chunk->size) - SIZE_T_SIZE);
	*footer_size = CHUNK_SIZE_WITHOUT_FLAGS(freed_chunk->size);
}

void free_tiny_chunk(size_t *ptr_to_chunk)
{
	free_chunk_header_t *freed_chunk = (free_chunk_header_t *)ptr_to_chunk;
	heap_g.tiny_bin_head->prev = freed_chunk;
	freed_chunk->next = heap_g.tiny_bin_head;
	freed_chunk->prev = NULL;
	freed_chunk->size &= ~IN_USE;
	heap_g.tiny_bin_head = freed_chunk;
}
