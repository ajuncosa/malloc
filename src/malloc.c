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

	printf("mallocing %zu bytes\n", size);
	size_t chunk_size = ALIGN(size + SIZE_T_SIZE);
	printf("chunk size: %zu bytes\n", chunk_size);

	/* LARGE ALLOCATION */
	if (size > SMALL_ZONE_CHUNK_MAX_SIZE)
	{
		zone_t *new_zone = mmap(NULL, chunk_size + ALIGN(sizeof(zone_t)), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if (new_zone == MAP_FAILED)
			return NULL;

		new_zone->next = NULL;
		new_zone->prev = NULL;

		size_t *new_chunk = (size_t *)((uint8_t *)new_zone + ALIGN(sizeof(zone_t)));
		*new_chunk = chunk_size | IN_USE | MMAPPED;

		// insert new allocated chunk at the front of the large list:
		if (heap_g.large_zones_head != NULL)
		{
			new_zone->next = heap_g.large_zones_head;
			zone_t *large_head = (zone_t *)(heap_g.large_zones_head);
			large_head->prev = new_zone;
		}
		heap_g.large_zones_head = new_zone;

		ptr = (uint8_t *)new_chunk + SIZE_T_SIZE;
	}
	/* TINY ALLOCATION */
	else if (size <= TINY_ZONE_CHUNK_MAX_SIZE)
	{
		printf("TINY ALLOCATION\n");
		/*
		size_t chunk_size = ALIGN(size + SIZE_T_SIZE);
		//size_t chunk_size = ALIGN(size + ALIGN(sizeof(inuse_chunk_header_t)));
		chunk_size = chunk_size < MIN_CHUNK_SIZE ? MIN_CHUNK_SIZE : chunk_size;

		size_t *chunk_header = mmap(NULL, chunk_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		*chunk_header = chunk_size | 1; // allocated bit
		return (char *)chunk_header + SIZE_T_SIZE;
		*/
	}
	/* SMALL ALLOCATION */
	else
	{
		printf("SMALL ALLOCATION\n");
	}

	printf("data address: %p\n", ptr);
	
	//munmap(ptr, size);
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
	if ((*ptr_to_chunk & MMAPPED) == MMAPPED)
	{
		printf("Freeing a LARGE chunk (and zone).\n");
		zone_t *ptr_to_zone = (zone_t *)((uint8_t *)ptr_to_chunk - ALIGN(sizeof(zone_t)));
		printf("ptr to zone: %p, next: %p, prev: %p\n", ptr_to_zone, ptr_to_zone->next, ptr_to_zone->prev);
		if (ptr_to_zone == heap_g.large_zones_head)
			heap_g.large_zones_head = ptr_to_zone->next;
		if (ptr_to_zone->prev != NULL)
			ptr_to_zone->prev->next = ptr_to_zone->next;
		if (ptr_to_zone->next != NULL)
			ptr_to_zone->next->prev = ptr_to_zone->prev;
		munmap(ptr_to_zone, ALIGN(*ptr_to_chunk) + ALIGN(sizeof(zone_t)));
		return;
	}
}

// TODO: print size allocated by the user instead of chunk size
// TODO: sort by address (small to big)
// TODO: remove all printfs
void show_alloc_mem(void)
{
	size_t total_bytes = 0;

	printf("TINY: %p\n", heap_g.tiny_zones_head);

	printf("\nSMALL: %p\n", heap_g.small_zones_head);

	printf("\nLARGE: %p\n", heap_g.large_zones_head);
	for (zone_t *large_zone = heap_g.large_zones_head; large_zone != NULL; large_zone = large_zone->next)
	{
		size_t *chunk_ptr = (size_t *)((uint8_t *)large_zone + ALIGN(sizeof(zone_t)));
		printf("  %p - %p: %zu bytes\n",
			chunk_ptr,
			(uint8_t *)chunk_ptr + *chunk_ptr, *chunk_ptr);
		total_bytes += *chunk_ptr;
	}

	printf("\nTotal: %zu bytes\n", total_bytes);
}