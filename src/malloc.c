#include <stdio.h>
#include <sys/mman.h>

#include "malloc.h"
#include "heap.h"

void *malloc(size_t size)
{
	static bool heap_initialized = false;
	if (heap_initialized == false)
	{
		if (init_heap() == false)
			return NULL;

		heap_initialized = true;
	}
	void *ptr = NULL;

	//printf("mallocing %zu bytes\n", size);
	size_t chunk_size = ALIGN(size + SIZE_T_SIZE);
	//printf("chunk size: %zu bytes\n", chunk_size);

	/* LARGE ALLOCATION */
	if (chunk_size > SMALL_ZONE_CHUNK_MAX_SIZE)
	{
		//printf("LARGE ALLOCATION\n");
		if ((ptr = allocate_large_chunk(chunk_size)) == NULL)
			return NULL;
	}
	/* TINY ALLOCATION */
	else if (chunk_size <= TINY_ZONE_CHUNK_MAX_SIZE)
	{
		//printf("TINY ALLOCATION\n");
		if ((ptr = allocate_tiny_chunk()) == NULL)
			return NULL;
	}
	/* SMALL ALLOCATION */
	else
	{
		//printf("SMALL ALLOCATION\n");
		if ((ptr = allocate_small_chunk(chunk_size)) == NULL)
			return NULL;
	}

	//printf("data address: %p\n", ptr);
	
	return ptr;
}

// arena, heap, bin (small = all the same size; large = range of sizes), free chunk


void free(void *ptr)
{
	size_t *ptr_to_chunk = (size_t *)((uint8_t *)ptr - SIZE_T_SIZE);
	if ((*ptr_to_chunk & IN_USE) == 0)
	{
		printf("Error: the pointer you are trying to free is not allocated.\n");
		exit(1);
	}
	if (CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) > SMALL_ZONE_CHUNK_MAX_SIZE)
	{
		printf("Freeing a LARGE chunk (and zone).\n");
		free_large_chunk(ptr_to_chunk);
	}
	else if (CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) == TINY_ZONE_CHUNK_MAX_SIZE)
	{
		printf("Freeing a TINY chunk.\n");
		free_tiny_chunk(ptr_to_chunk);
	}
	else if (CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) > TINY_ZONE_CHUNK_MAX_SIZE
		&& CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) <= SMALL_ZONE_CHUNK_MAX_SIZE)
	{
		printf("Freeing a SMALL chunk.\n");
		free_small_chunk(ptr_to_chunk);
	}
	else
	{
		printf("Error: something does not match when attempting to free the memory. Memory might be corrupt or you might be trying to free an invalid pointer.\n");
		exit(1);
	}
}

// TODO: print size allocated by the user instead of chunk size
// TODO: sort by address (small to big)
// TODO: remove all printfs
void show_alloc_mem(void)
{
	size_t total_bytes = 0;

	printf("TINY: %p\n", heap_g.tiny_zones_head);
	for (zone_header_t *tiny_zone = heap_g.tiny_zones_head; tiny_zone != NULL; tiny_zone = tiny_zone->next)
	{
		size_t *chunk_ptr = (size_t *)((uint8_t *)tiny_zone + ZONE_HEADER_T_SIZE);
		size_t chunk_size = CHUNK_SIZE_WITHOUT_FLAGS(*chunk_ptr);
		for (size_t i = 0; i < (TINY_ZONE_SIZE / TINY_ZONE_CHUNK_MAX_SIZE); i++)
    	{
			if ((*chunk_ptr & IN_USE) == IN_USE)
			{
				printf("  %p - %p: %zu bytes\n", chunk_ptr + SIZE_T_SIZE, (uint8_t *)chunk_ptr + chunk_size, chunk_size);
				total_bytes += chunk_size;
			}
        	chunk_ptr = (size_t *)((uint8_t *)(chunk_ptr) + chunk_size);
		}
	}

	printf("\nSMALL: %p\n", heap_g.small_zones_head);
	for (zone_header_t *small_zone = heap_g.small_zones_head; small_zone != NULL; small_zone = small_zone->next)
	{
		size_t *chunk_ptr = (size_t *)((uint8_t *)small_zone + ZONE_HEADER_T_SIZE);
		size_t chunk_size;
		for (size_t i = 0; i < SMALL_ZONE_SIZE; i += chunk_size)
    	{
			chunk_size = CHUNK_SIZE_WITHOUT_FLAGS(*chunk_ptr);
			if ((*chunk_ptr & IN_USE) == IN_USE)
			{
				printf("  %p - %p: %zu bytes\n", chunk_ptr + SIZE_T_SIZE, (uint8_t *)chunk_ptr + chunk_size, chunk_size);
				total_bytes += chunk_size;
			}
        	chunk_ptr = (size_t *)((uint8_t *)(chunk_ptr) + chunk_size);
		}
	}

	printf("\nLARGE: %p\n", heap_g.large_zones_head);
	for (zone_header_t *large_zone = heap_g.large_zones_head; large_zone != NULL; large_zone = large_zone->next)
	{
		size_t *chunk_ptr = (size_t *)((uint8_t *)large_zone + ZONE_HEADER_T_SIZE);
		size_t chunk_size = CHUNK_SIZE_WITHOUT_FLAGS(*chunk_ptr);
		printf("  %p - %p: %zu bytes\n", chunk_ptr + SIZE_T_SIZE, (uint8_t *)chunk_ptr + chunk_size, chunk_size);
		total_bytes += chunk_size;
	}

	printf("\nTotal: %zu bytes\n", total_bytes);
}
