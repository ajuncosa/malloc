#include <stdio.h>
#include <sys/mman.h>
#include "malloc.h"
#include "heap.h"
#include "utils.h"

// TODO: use getrlimit
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

	size_t aligned_requested_size = ALIGN(size + SIZE_T_SIZE);

	/* LARGE ALLOCATION */
	if (aligned_requested_size > heap_g.small_zone_chunk_max_size)
		return allocate_large_chunk(aligned_requested_size);
	/* TINY ALLOCATION */
	else if (aligned_requested_size <= heap_g.tiny_zone_chunk_max_size)
		return allocate_tiny_chunk();
	/* SMALL ALLOCATION */
	else
		return allocate_small_chunk(aligned_requested_size);
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

	size_t new_chunk_size = ALIGN(size + SIZE_T_SIZE);
	if (new_chunk_size == CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk))
		return ptr;

	/* LARGE REALLOC */
	if (CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) > heap_g.small_zone_chunk_max_size)
		return realloc_large_chunk(ptr, ptr_to_chunk, size);
	/* TINY REALLOC */
	else if (CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) == heap_g.tiny_zone_chunk_max_size)
		return realloc_tiny_chunk(ptr, ptr_to_chunk, size, new_chunk_size);
	/* SMALL REALLOC */
	else if (CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) > heap_g.tiny_zone_chunk_max_size
		&& CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) <= heap_g.small_zone_chunk_max_size)
		return realloc_small_chunk(ptr, ptr_to_chunk, size, new_chunk_size);
	
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
		size_t chunk_size = CHUNK_SIZE_WITHOUT_FLAGS(*chunk_ptr);
		for (size_t i = 0; i < ((heap_g.tiny_zone_size - ZONE_HEADER_T_SIZE) / heap_g.tiny_zone_chunk_max_size); i++)
    	{
			if ((*chunk_ptr & IN_USE) == IN_USE)
			{
				// print_str("  ");
				// print_address_hex(chunk_ptr);
				// print_str(" - ");
				// print_address_hex((uint8_t *)chunk_ptr + chunk_size);
				// print_str(": ");
				printf("  %p - %p: %zu bytes\n", (uint8_t *)chunk_ptr, (uint8_t *)chunk_ptr + chunk_size, chunk_size);
				total_bytes += chunk_size;
			}
        	chunk_ptr = (size_t *)((uint8_t *)(chunk_ptr) + chunk_size);
		}
	}

	for (zone_header_t *small_zone = get_zone_list_last(heap_g.small_zones_head); small_zone != NULL; small_zone = small_zone->prev)
	{
		printf("\nSMALL: %p\n", small_zone);
		size_t *chunk_ptr = (size_t *)((uint8_t *)small_zone + ZONE_HEADER_T_SIZE);
		size_t chunk_size;
		for (size_t i = 0; i < (heap_g.small_zone_size - ZONE_HEADER_T_SIZE); i += chunk_size)
    	{
			chunk_size = CHUNK_SIZE_WITHOUT_FLAGS(*chunk_ptr);
			if ((*chunk_ptr & IN_USE) == IN_USE)
			{
				printf("  %p - %p: %zu bytes\n", (uint8_t *)chunk_ptr, (uint8_t *)chunk_ptr + chunk_size, chunk_size);
				total_bytes += chunk_size;
			}
        	chunk_ptr = (size_t *)((uint8_t *)(chunk_ptr) + chunk_size);
		}
	}

	for (zone_header_t *large_zone = get_zone_list_last(heap_g.large_zones_head); large_zone != NULL; large_zone = large_zone->prev)
	{
		printf("\nLARGE: %p\n", large_zone);
		size_t *chunk_ptr = (size_t *)((uint8_t *)large_zone + ZONE_HEADER_T_SIZE);
		size_t chunk_size = CHUNK_SIZE_WITHOUT_FLAGS(*chunk_ptr);
		printf("  %p - %p: %zu bytes\n", (uint8_t *)chunk_ptr, (uint8_t *)chunk_ptr + chunk_size, chunk_size);
		total_bytes += chunk_size;
	}

	printf("\nTotal: %zu bytes\n", total_bytes);
}
