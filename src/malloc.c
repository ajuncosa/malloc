#include <stdio.h>
#include <sys/mman.h>
#include <string.h> // FIXME: remove

#include "malloc.h"
#include "heap.h"

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

	//printf("mallocing %zu bytes\n", size);
	size_t chunk_size = ALIGN(size + SIZE_T_SIZE);
	//printf("chunk size: %zu bytes\n", chunk_size);

	/* LARGE ALLOCATION */
	if (chunk_size > SMALL_ZONE_CHUNK_MAX_SIZE)
		return allocate_large_chunk(chunk_size);
	/* TINY ALLOCATION */
	else if (chunk_size <= TINY_ZONE_CHUNK_MAX_SIZE)
		return allocate_tiny_chunk();
	/* SMALL ALLOCATION */
	else
		return allocate_small_chunk(chunk_size);

	//printf("data address: %p\n", ptr);
}

void free(void *ptr)
{
	if (ptr == NULL)
		return;

	size_t *ptr_to_chunk = (size_t *)((uint8_t *)ptr - SIZE_T_SIZE);
	if ((*ptr_to_chunk & IN_USE) == 0)
	{
		printf("Error: the pointer you are trying to free is not allocated.\n");
		exit(1);
	}

	/* LARGE FREE */
	if (CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) > SMALL_ZONE_CHUNK_MAX_SIZE)
		free_large_chunk(ptr_to_chunk);
	/* TINY FREE */
	else if (CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) == TINY_ZONE_CHUNK_MAX_SIZE)
		free_tiny_chunk(ptr_to_chunk);
	/* SMALL FREE */
	else if (CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) > TINY_ZONE_CHUNK_MAX_SIZE
		&& CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) <= SMALL_ZONE_CHUNK_MAX_SIZE)
		free_small_chunk(ptr_to_chunk);
	else
	{
		printf("Error: something does not match when attempting to free the memory. Memory might be corrupt or you might be trying to free an invalid pointer.\n");
		exit(1);
	}
}

void *realloc(void *ptr, size_t size)
{
	if (ptr == NULL)
		return malloc(size);

	if (size == 0)
		return NULL;

	size_t *ptr_to_chunk = (size_t *)((uint8_t *)ptr - SIZE_T_SIZE);
	if ((*ptr_to_chunk & IN_USE) == 0)
	{
		printf("Error: the pointer you are trying to realloc is not allocated.\n");
		exit(1);
	}

	size_t realloc_chunk_size = ALIGN(size + SIZE_T_SIZE);
	printf("realloc chunk size: %zu bytes\n", realloc_chunk_size);

	if (realloc_chunk_size == CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk))
	{
		printf("Reallocing same size (returning same ptr).\n");
		return ptr;
	}

	void *new_ptr = NULL;
	size_t copy_size = size < CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) ? size : CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk);

	/* LARGE REALLOC */
	if (CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) > SMALL_ZONE_CHUNK_MAX_SIZE)
	{
		printf("Reallocing a LARGE chunk.\n");
		new_ptr = malloc(size);
		// FIXME: change this for the libft version:
		memcpy(new_ptr, ptr, copy_size);
		free_large_chunk(ptr_to_chunk);
	}
	/* TINY REALLOC */
	else if (CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) == TINY_ZONE_CHUNK_MAX_SIZE)
	{
		printf("Reallocing a TINY chunk.\n");
		if (realloc_chunk_size <= TINY_ZONE_CHUNK_MAX_SIZE)
		{
		printf("Returning same ptr.\n");
			return ptr;
		}

		new_ptr = malloc(size);
		// FIXME: change this for the libft version:
		memcpy(new_ptr, ptr, copy_size);
		free_tiny_chunk(ptr_to_chunk);
	}
	/* SMALL REALLOC */
	else if (CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) > TINY_ZONE_CHUNK_MAX_SIZE
		&& CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) <= SMALL_ZONE_CHUNK_MAX_SIZE)
	{
		printf("Reallocing a SMALL chunk.\n");
		if (realloc_chunk_size <= TINY_ZONE_CHUNK_MAX_SIZE
			|| realloc_chunk_size > SMALL_ZONE_CHUNK_MAX_SIZE)
		{
		printf("Returning tiny or large.\n");
			new_ptr = malloc(size);
			// FIXME: change this for the libft version:
			memcpy(new_ptr, ptr, copy_size);
			free_small_chunk(ptr_to_chunk);
		}
		else if (realloc_chunk_size <= CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk))
		{
		printf("Attempting to split.\n");
			// If the remainder of splitting would be big enough to store
        	// more small chunks, split the chunk:
			free_chunk_header_t *remaining_chunk = try_split_chunk(ptr_to_chunk, realloc_chunk_size);
			if (remaining_chunk != NULL)
			{
		printf("Chunk has been split.\n");
				add_chunk_to_small_bin(remaining_chunk);
				*ptr_to_chunk |= IN_USE;
			}
		printf("Returning same ptr.\n");
			return ptr;
		}
		else
		{
		printf("attempting to coalesce.\n");
			// TODO:
			//free_chunk_header_t *coalesced = coalesce(ptr_to_chunk);
			free_chunk_header_t *coalesced = (free_chunk_header_t*)ptr_to_chunk;
			if (coalesced->size < realloc_chunk_size)
			{
				new_ptr = malloc(size);
				// FIXME: change this for the libft version:
				memcpy(new_ptr, ptr, copy_size);
				free_small_chunk(ptr_to_chunk);
			}
		}
	}
	else
	{
		printf("Error: something does not match when attempting to realloc the memory. Memory might be corrupt or you might be trying to realloc an invalid pointer.\n");
		exit(1);
	}
	return new_ptr;
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
