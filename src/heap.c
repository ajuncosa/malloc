#include <sys/mman.h>
#include "heap.h"

// global
heap_t heap_g;

bool init_heap(void)
{
	//size_t initial_tiny_zones_size = TINY_ZONE_SIZE + sizeof(zone_t);
	//size_t initial_small_zones_size = SMALL_ZONE_SIZE + sizeof(zone_t);
	//size_t initial_large_zones_size = sizeof(zone_t);

	heap_g.tiny_zones_head = mmap(NULL, TINY_ZONE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (heap_g.tiny_zones_head == MAP_FAILED)
		return false;
	//(zone_t *)(heap_g.tiny_zones_head)->remaining_free_bytes = TINY_ZONE_SIZE;
	//(zone_t*)(heap_g.tiny_zones_head)->prev_zone = NULL;
	//(zone_t*)(heap_g.tiny_zones_head)->next_zone = NULL;
	//(zone_t*)(heap_g.tiny_zones_head)->bin = NULL;

	heap_g.small_zones_head = mmap(NULL, SMALL_ZONE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (heap_g.small_zones_head == MAP_FAILED)
		return false;

	heap_g.large_zones_head = NULL;

	return true;
}
