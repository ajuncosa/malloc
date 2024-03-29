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


	if (size > SMALL_ZONE_CHUNK_MAX_SIZE)
	{
		size_t chunk_size = ALIGN(size + ALIGN(sizeof(mmapped_chunk_header_t)));
		printf("chunk size: %zu bytes\n", chunk_size);

		mmapped_chunk_header_t *new_chunk = mmap(NULL, chunk_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if (new_chunk == MAP_FAILED)
			return NULL;

		new_chunk->size = chunk_size;
		new_chunk->next = NULL;
		new_chunk->prev = NULL;

		// insert new allocated chunk at the front of the large list:
		if (heap_g.large_zones_head != NULL)
			new_chunk->next = heap_g.large_zones_head;
		heap_g.large_zones_head = new_chunk;

		ptr = (void*)new_chunk + ALIGN(sizeof(mmapped_chunk_header_t));
	}

	/*if (size <= TINY_ZONE_CHUNK_MAX_SIZE)
	{
		size_t chunk_size = ALIGN(size + SIZE_T_SIZE);
		//size_t chunk_size = ALIGN(size + ALIGN(sizeof(inuse_chunk_header_t)));
		chunk_size = chunk_size < MIN_CHUNK_SIZE ? MIN_CHUNK_SIZE : chunk_size;

		size_t *chunk_header = mmap(NULL, chunk_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		*chunk_header = chunk_size | 1; // allocated bit
		return (char *)chunk_header + SIZE_T_SIZE;
	}

	printf("address: %p\n", ptr);
	*/
	
	//munmap(ptr, size);
	return ptr;
}

// arena, heap, bin (small = all the same size; large = range of sizes), free chunk

// TODO: print size allocated by the user instead of chunk size
// TODO: remove all printfs
void show_alloc_mem(void)
{
	size_t total_bytes = 0;

	printf("TINY: %p\n", heap_g.tiny_zones_head);

	printf("\nSMALL: %p\n", heap_g.small_zones_head);

	printf("\nLARGE: %p\n", heap_g.large_zones_head);
	for (mmapped_chunk_header_t *chunk = heap_g.large_zones_head; chunk != NULL; chunk = chunk->next)
	{
		printf("  %p - %p: %zu bytes\n",
			chunk + ALIGN(sizeof(mmapped_chunk_header_t)),
			(uint8_t *)chunk + chunk->size, chunk->size);
		total_bytes += chunk->size;
	}

	printf("\nTotal: %zu bytes\n", total_bytes);
}