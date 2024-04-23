#include <stdio.h>
#include <sys/mman.h>
#include <errno.h>

#include "malloc.h"
#include "heap.h"

static void *allocate_large_chunk(size_t chunk_size);
static void *allocate_small_chunk(size_t chunk_size);
static void *allocate_tiny_chunk(void);

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
		//printf("Freeing a LARGE chunk (and zone).\n");
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
		return;
	}
	else if (CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) == TINY_ZONE_CHUNK_MAX_SIZE)
	{
		//printf("Freeing a TINY chunk.\n");
		free_chunk_header_t *freed_chunk = (free_chunk_header_t *)ptr_to_chunk;
		heap_g.tiny_bin_head->prev = freed_chunk;
		freed_chunk->next = heap_g.tiny_bin_head;
		freed_chunk->prev = NULL;
		freed_chunk->size &= ~IN_USE;
		heap_g.tiny_bin_head = freed_chunk;
	}
	else if (CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) > TINY_ZONE_CHUNK_MAX_SIZE
		&& CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) <= SMALL_ZONE_CHUNK_MAX_SIZE)
	{
		printf("Freeing a SMALL chunk.\n");
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

static void *allocate_large_chunk(size_t chunk_size)
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

static void *allocate_small_chunk(size_t chunk_size)
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
		printf("zone is full.\n");
		// the zone is full or there is not a big enough free chunk
		// TODO: handle
		return NULL;
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

static void *allocate_tiny_chunk(void)
{
	free_chunk_header_t *new_chunk = heap_g.tiny_bin_head;
	heap_g.tiny_bin_head = new_chunk->next;
	if (new_chunk->next != NULL)
		new_chunk->next->prev = NULL;
	new_chunk->prev = NULL;
	new_chunk->next = NULL;
	new_chunk->size |= IN_USE;
	return (uint8_t *)new_chunk + SIZE_T_SIZE;
}