#include <sys/mman.h>
#include <stdlib.h>
#include <errno.h>
#include "heap.h"
#include "utils.h"

// global
heap_t heap_g = { .tiny_zone_size = 0,
                  .small_zone_size = 0,
                  .tiny_zone_chunk_max_size = 0,
                  .small_zone_chunk_max_size = 0,
                  .tiny_zones_head = NULL,
                  .small_zones_head = NULL,
                  .large_zones_head = NULL ,
                  .tiny_bin_head = NULL,
                  .small_bin_head = NULL,
                  .small_unsorted_list_head = NULL };

bool init_heap(void)
{
    heap_g.tiny_zone_size = getpagesize();
    heap_g.small_zone_size = getpagesize() * 800;
    heap_g.tiny_zone_chunk_max_size = heap_g.tiny_zone_size / 128;
    heap_g.small_zone_chunk_max_size = heap_g.small_zone_size / 100;
    if (heap_g.tiny_zone_chunk_max_size < MIN_FREE_CHUNK_SIZE) // just in case
        return false;
    return true;
}

zone_header_t *allocate_new_tiny_zone(void)
{
    zone_header_t *new_tiny_zone = mmap(NULL, heap_g.tiny_zone_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (new_tiny_zone == MAP_FAILED)
		return NULL;

    new_tiny_zone->prev = NULL;
    new_tiny_zone->next = heap_g.tiny_zones_head;
    if (heap_g.tiny_zones_head != NULL)
        heap_g.tiny_zones_head->prev = new_tiny_zone;
    heap_g.tiny_zones_head = new_tiny_zone;

    /* Initialize tiny zone bin: */
    free_chunk_header_t *initial_tiny_free_chunk = (free_chunk_header_t *)((uint8_t *)(new_tiny_zone) + ZONE_HEADER_T_SIZE);
    initial_tiny_free_chunk->size = heap_g.tiny_zone_chunk_max_size;
    initial_tiny_free_chunk->prev = NULL;
    initial_tiny_free_chunk->next = NULL;
    heap_g.tiny_bin_head = initial_tiny_free_chunk;

    free_chunk_header_t *next_free_chunk = NULL;
    for (size_t i = 0; i < ((heap_g.tiny_zone_size - ZONE_HEADER_T_SIZE) / heap_g.tiny_zone_chunk_max_size) - 1; i++)
    {
        next_free_chunk = (free_chunk_header_t *)((uint8_t *)(initial_tiny_free_chunk) + initial_tiny_free_chunk->size);
        next_free_chunk->size = heap_g.tiny_zone_chunk_max_size;
        next_free_chunk->prev = initial_tiny_free_chunk;
        next_free_chunk->next = NULL;
        initial_tiny_free_chunk->next = next_free_chunk;
        initial_tiny_free_chunk = next_free_chunk;
    }

    return new_tiny_zone;
}

zone_header_t *allocate_new_small_zone(void)
{
    zone_header_t *new_small_zone = mmap(NULL, heap_g.small_zone_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (new_small_zone == MAP_FAILED)
		return NULL;

    new_small_zone->prev = NULL;
    new_small_zone->next = heap_g.small_zones_head;
    if (heap_g.small_zones_head != NULL)
        heap_g.small_zones_head->prev = new_small_zone;
    heap_g.small_zones_head = new_small_zone;

    /* Initialize small zone bin: */
    free_chunk_header_t *initial_small_free_chunk = (free_chunk_header_t *)((uint8_t *)(new_small_zone) + ZONE_HEADER_T_SIZE);
    initial_small_free_chunk->size = heap_g.small_zone_size - ZONE_HEADER_T_SIZE;
    initial_small_free_chunk->prev = NULL;
    initial_small_free_chunk->next = NULL;
    heap_g.small_bin_head = initial_small_free_chunk;
    set_chunk_footer_size(initial_small_free_chunk);

    return new_small_zone;
}

void free_small_zone(zone_header_t *ptr_to_zone)
{
    remove_zone_from_list(&heap_g.small_zones_head, ptr_to_zone);
	munmap(ptr_to_zone, heap_g.small_zone_size);
}

void free_tiny_zone(zone_header_t *ptr_to_zone)
{
    size_t *chunk = (size_t *)((uint8_t *)ptr_to_zone + ZONE_HEADER_T_SIZE);
    size_t number_of_tiny_chunks_per_zone = ((heap_g.tiny_zone_size - ZONE_HEADER_T_SIZE) / heap_g.tiny_zone_chunk_max_size);
    for (size_t i = 0; i < number_of_tiny_chunks_per_zone; i++)
    {
        if ((*chunk & IN_USE) == 0)
            remove_chunk_from_list(&heap_g.tiny_bin_head, (free_chunk_header_t*)chunk);
        chunk = (size_t *)((uint8_t *)chunk + CHUNK_SIZE_WITHOUT_FLAGS(*chunk));
    }

    remove_zone_from_list(&heap_g.tiny_zones_head, ptr_to_zone);
	munmap(ptr_to_zone, heap_g.tiny_zone_size);
}

zone_header_t *get_zone(void *chunk_ptr, zone_header_t *zone_list_head, size_t zone_size)
{
    for (zone_header_t *zone = zone_list_head; zone != NULL; zone = zone->next)
    {
        if (chunk_ptr > (void *)zone && chunk_ptr < (void *)((uint8_t *)zone + zone_size))
        {
            return zone;
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
		heap_g.large_zones_head->prev = new_zone;
	}
	heap_g.large_zones_head = new_zone;

	return (uint8_t *)new_chunk + SIZE_T_SIZE;
}

void *allocate_small_chunk(size_t chunk_size)
{
	free_chunk_header_t *new_chunk = NULL;

    free_chunk_header_t *unsorted_chunk = heap_g.small_unsorted_list_head;
    free_chunk_header_t *next_unsorted_chunk = NULL;
	while (unsorted_chunk != NULL)
	{
		if (CHUNK_SIZE_WITHOUT_FLAGS(unsorted_chunk->size) == chunk_size)
		{
			new_chunk = unsorted_chunk;
            remove_chunk_from_list(&heap_g.small_unsorted_list_head, new_chunk);
			break;
		}

        // Deferred coalescing
        free_chunk_header_t *coalesced_chunk = coalesce_backward(unsorted_chunk);
        if (unsorted_chunk != coalesced_chunk)
        {
            replace_chunk_in_list(&heap_g.small_unsorted_list_head, unsorted_chunk, coalesced_chunk);
            unsorted_chunk = coalesced_chunk;
        }
        coalesce_forward((size_t *)unsorted_chunk);
        next_unsorted_chunk = unsorted_chunk->next;
        
        if (CHUNK_SIZE_WITHOUT_FLAGS(unsorted_chunk->size) == chunk_size)
        {
            new_chunk = unsorted_chunk;
            remove_chunk_from_list(&heap_g.small_unsorted_list_head, new_chunk);
            break;
        }

        // If the zone is empty and there are chunks available in another zone,
        // free the zone (otherwise, we will need to immediately allocate a new 
        // one after this anyway)
        if (CHUNK_SIZE_WITHOUT_FLAGS(unsorted_chunk->size) == (heap_g.small_zone_size - ZONE_HEADER_T_SIZE)
            && (next_unsorted_chunk != NULL
            || (heap_g.small_bin_head != NULL && CHUNK_SIZE_WITHOUT_FLAGS(get_chunk_list_last(heap_g.small_bin_head)->size) >= chunk_size)))
        {
            zone_header_t *zone = get_zone(unsorted_chunk, heap_g.small_zones_head, heap_g.small_zone_size);
            remove_chunk_from_list(&heap_g.small_unsorted_list_head, unsorted_chunk);
            free_small_zone(zone);
        }
        else
        {
            remove_chunk_from_list(&heap_g.small_unsorted_list_head, unsorted_chunk);
            add_chunk_to_small_bin(unsorted_chunk);
        }
            
        unsorted_chunk = next_unsorted_chunk;
	}
    if (new_chunk == NULL)
    {
	    for (free_chunk_header_t *chunk = heap_g.small_bin_head; chunk != NULL; chunk = chunk->next)
        {
            if (CHUNK_SIZE_WITHOUT_FLAGS(chunk->size) >= chunk_size)
            {
                new_chunk = chunk;
                break;
            }
        }
        if (new_chunk ==  NULL)
        {
            // the zone is full or there is not a big enough free chunk
            if (allocate_new_small_zone() == NULL)
                return NULL;
            new_chunk = heap_g.small_bin_head;
        }

        // If the remainder of splitting would be big enough to store
        // more small chunks, split the chunk:
        free_chunk_header_t *remaining_chunk = try_split_chunk((size_t *)new_chunk, chunk_size);
        if (remaining_chunk != NULL)
            replace_chunk_in_list(&heap_g.small_bin_head, new_chunk, remaining_chunk);
        else
        {
            remove_chunk_from_list(&heap_g.small_bin_head, new_chunk);
            zone_header_t *new_chunk_zone = get_zone(new_chunk, heap_g.small_zones_head, heap_g.small_zone_size);
            size_t *next_chunk = (size_t *)((uint8_t *)new_chunk + CHUNK_SIZE_WITHOUT_FLAGS(new_chunk->size));
            if (next_chunk != (size_t *)((uint8_t *)new_chunk_zone + heap_g.small_zone_size))
                *next_chunk &= ~PREVIOUS_FREE;
        }
    }
	new_chunk->size |= IN_USE;

	return (uint8_t *)new_chunk + SIZE_T_SIZE;
}

void *allocate_tiny_chunk(void)
{
	free_chunk_header_t *new_chunk = heap_g.tiny_bin_head;
	if (new_chunk == NULL)
	{
		if (allocate_new_tiny_zone() == NULL)
        	return NULL;
		new_chunk = heap_g.tiny_bin_head;
	}
    remove_chunk_from_list(&heap_g.tiny_bin_head, new_chunk);
	new_chunk->size |= IN_USE;

	return (uint8_t *)new_chunk + SIZE_T_SIZE;
}

void free_large_chunk(size_t *ptr_to_chunk)
{
	zone_header_t *ptr_to_zone = (zone_header_t *)((uint8_t *)ptr_to_chunk - ZONE_HEADER_T_SIZE);
    remove_zone_from_list(&heap_g.large_zones_head, ptr_to_zone);
	munmap(ptr_to_zone, CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) + ZONE_HEADER_T_SIZE);
}

void free_small_chunk(size_t *ptr_to_chunk)
{
	free_chunk_header_t *freed_chunk = (free_chunk_header_t *)ptr_to_chunk;
    zone_header_t *freed_chunk_zone = get_zone(freed_chunk, heap_g.small_zones_head, heap_g.small_zone_size);
	size_t *next_chunk = (size_t *)((uint8_t *)freed_chunk + CHUNK_SIZE_WITHOUT_FLAGS(freed_chunk->size));
	if (next_chunk != (size_t *)((uint8_t *)freed_chunk_zone + heap_g.small_zone_size))
		*next_chunk |= PREVIOUS_FREE;

    add_chunk_to_list_front(&heap_g.small_unsorted_list_head, freed_chunk);
	freed_chunk->size &= ~IN_USE;
    set_chunk_footer_size(freed_chunk);
}

void free_tiny_chunk(size_t *ptr_to_chunk)
{
	free_chunk_header_t *freed_chunk = (free_chunk_header_t *)ptr_to_chunk;
	freed_chunk->size &= ~IN_USE;
    add_chunk_to_list_front(&heap_g.tiny_bin_head, freed_chunk);

    // Check if zone is empty and, if it is and there are more zones available,
    // free the zone:
    zone_header_t *chunk_zone = get_zone(ptr_to_chunk, heap_g.tiny_zones_head, heap_g.tiny_zone_size);
    size_t *chunk = (size_t *)((uint8_t *)chunk_zone + ZONE_HEADER_T_SIZE);
    size_t free_chunks_in_zone = 0;
    size_t number_of_tiny_chunks_per_zone = ((heap_g.tiny_zone_size - ZONE_HEADER_T_SIZE) / heap_g.tiny_zone_chunk_max_size);
    while (free_chunks_in_zone < number_of_tiny_chunks_per_zone && (*chunk & IN_USE) == 0)
    {
        free_chunks_in_zone++;
        chunk = (size_t *)((uint8_t *)chunk + CHUNK_SIZE_WITHOUT_FLAGS(*chunk));
    }

    if (free_chunks_in_zone == number_of_tiny_chunks_per_zone
        && (chunk_zone->prev != NULL || chunk_zone->next != NULL))
    {
        free_tiny_zone(chunk_zone);
        return;
    }
}

void *realloc_large_chunk(void *ptr_to_data, size_t *ptr_to_chunk, size_t new_alloc_size)
{
	void *new_ptr = NULL;
    size_t old_chunk_data_size = CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) - SIZE_T_SIZE;
	size_t copy_size = new_alloc_size < old_chunk_data_size ? new_alloc_size : old_chunk_data_size;

    if ((new_ptr = malloc(new_alloc_size)) == NULL)
        return NULL;

    ft_memcpy(new_ptr, ptr_to_data, copy_size);
    free_large_chunk(ptr_to_chunk);

    return new_ptr;
}

void *realloc_small_chunk(void *ptr_to_data, size_t *ptr_to_chunk, size_t new_alloc_size, size_t new_chunk_size)
{
	void *new_ptr = NULL;
    size_t old_chunk_data_size = CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) - SIZE_T_SIZE;
	size_t copy_size = new_alloc_size < old_chunk_data_size ? new_alloc_size : old_chunk_data_size;
	
    // The chunk resulting from the realloc will be "tiny" or "large"
    if (new_chunk_size <= heap_g.tiny_zone_chunk_max_size
        || new_chunk_size > heap_g.small_zone_chunk_max_size)
    {
        if ((new_ptr = malloc(new_alloc_size)) == NULL)
            return NULL;

        ft_memcpy(new_ptr, ptr_to_data, copy_size);
        free_small_chunk(ptr_to_chunk);
    }
    // The allocation is being reduced and the chunk resulting from the realloc will be "small"
    else if (new_chunk_size <= CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk))
    {
        // If the remainder of splitting would be big enough to store
        // more small chunks, split the chunk:
        free_chunk_header_t *remaining_chunk = try_split_chunk(ptr_to_chunk, new_chunk_size);
        if (remaining_chunk != NULL)
        {
            coalesce_forward((size_t *)remaining_chunk);
            add_chunk_to_small_bin(remaining_chunk);
        }
        return ptr_to_data;
    }
    // The allocation is growing and the chunk resulting from the realloc will be "small"
    else
    {
        coalesce_forward(ptr_to_chunk);

        // If the coalesced chunk is still too small, do a malloc-copy-free:
        if (CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) < new_chunk_size)
        {
            if ((new_ptr = malloc(new_alloc_size)) == NULL)
                return NULL;

            ft_memcpy(new_ptr, ptr_to_data, copy_size);
            free_small_chunk(ptr_to_chunk);
        }
        else
        {
            // If the remainder of splitting would be big enough to store
            // more small chunks, split the chunk:
            free_chunk_header_t *remaining_chunk = try_split_chunk(ptr_to_chunk, new_chunk_size);
            if (remaining_chunk != NULL)
            {
                coalesce_forward((size_t *)remaining_chunk);
                add_chunk_to_small_bin(remaining_chunk);
            }
            return ptr_to_data;
        }
    }
    return new_ptr;
}

void *realloc_tiny_chunk(void *ptr_to_data, size_t *ptr_to_chunk, size_t new_alloc_size, size_t new_chunk_size)
{
	void *new_ptr = NULL;
    size_t old_chunk_data_size = CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) - SIZE_T_SIZE;
	size_t copy_size = new_alloc_size < old_chunk_data_size ? new_alloc_size : old_chunk_data_size;

    if (new_chunk_size <= heap_g.tiny_zone_chunk_max_size)
        return ptr_to_data;

    if ((new_ptr = malloc(new_alloc_size)) == NULL)
        return NULL;

    ft_memcpy(new_ptr, ptr_to_data, copy_size);
    free_tiny_chunk(ptr_to_chunk);

    return new_ptr;
}

void add_chunk_to_small_bin(free_chunk_header_t *chunk)
{
    free_chunk_header_t *next_largest_chunk_in_small_bin = heap_g.small_bin_head;
    while (next_largest_chunk_in_small_bin != NULL && next_largest_chunk_in_small_bin->size < chunk->size)
    {
        next_largest_chunk_in_small_bin = next_largest_chunk_in_small_bin->next;
    }

    // add chunk to the small bin, ordered by size
    chunk->next = next_largest_chunk_in_small_bin;
    if (heap_g.small_bin_head == next_largest_chunk_in_small_bin)
        heap_g.small_bin_head = chunk;
    if (next_largest_chunk_in_small_bin != NULL)
    {
        chunk->prev = next_largest_chunk_in_small_bin->prev;
        if (next_largest_chunk_in_small_bin->prev != NULL)
            next_largest_chunk_in_small_bin->prev->next = chunk;
        next_largest_chunk_in_small_bin->prev = chunk;
    }
    else
        chunk->prev = NULL;
}

free_chunk_header_t *coalesce_backward(free_chunk_header_t *chunk)
{
    if (chunk->size & IN_USE)
        return chunk;

    size_t coalesced_chunk_size = CHUNK_SIZE_WITHOUT_FLAGS(chunk->size);
    free_chunk_header_t *coalesced_chunk = chunk;
    void *chunk_zone_begin = (uint8_t *)get_zone(chunk, heap_g.small_zones_head, heap_g.small_zone_size) + ZONE_HEADER_T_SIZE;

    // If PREVIOUS_FREE is not set, the previous chunk footer size_t is
    // not usable (those bytes might be in use by the user payload)
    bool prev_is_free_to_coalesce = ((void *)chunk > chunk_zone_begin) && ((chunk->size & PREVIOUS_FREE) == PREVIOUS_FREE);
    size_t prev_size = *(size_t *)((uint8_t *)chunk - SIZE_T_SIZE);
    size_t *prev_chunk_ptr = (size_t *)((uint8_t *)chunk - prev_size);
    while (prev_is_free_to_coalesce)
    {
        coalesced_chunk = (free_chunk_header_t *)prev_chunk_ptr;
        coalesced_chunk_size += CHUNK_SIZE_WITHOUT_FLAGS(*prev_chunk_ptr);
        remove_chunk_from_list(&heap_g.small_bin_head, (free_chunk_header_t *)prev_chunk_ptr);
        remove_chunk_from_list(&heap_g.small_unsorted_list_head, (free_chunk_header_t *)prev_chunk_ptr);
        prev_is_free_to_coalesce = ((void *)prev_chunk_ptr >= chunk_zone_begin) && ((*prev_chunk_ptr & PREVIOUS_FREE) == PREVIOUS_FREE);
        prev_size = *(size_t *)((uint8_t *)prev_chunk_ptr - SIZE_T_SIZE);
        prev_chunk_ptr = (size_t *)((uint8_t *)prev_chunk_ptr - prev_size);
    }
    coalesced_chunk->size = coalesced_chunk_size;
    set_chunk_footer_size(coalesced_chunk);

    return coalesced_chunk;
}

void coalesce_forward(size_t *chunk)
{
    size_t coalesced_chunk_size = *chunk;
    void *chunk_zone_begin = (uint8_t *)get_zone(chunk, heap_g.small_zones_head, heap_g.small_zone_size) + ZONE_HEADER_T_SIZE;
    void *chunk_zone_end = (uint8_t *)chunk_zone_begin + heap_g.small_zone_size - ZONE_HEADER_T_SIZE;

    size_t *next_chunk_ptr = (size_t *)((uint8_t *)chunk + CHUNK_SIZE_WITHOUT_FLAGS(*chunk));
    bool next_is_free_to_coalesce = ((void *)next_chunk_ptr < chunk_zone_end) && ((*next_chunk_ptr & IN_USE) == 0);
    while (next_is_free_to_coalesce)
    {
        coalesced_chunk_size += CHUNK_SIZE_WITHOUT_FLAGS(*next_chunk_ptr);
        remove_chunk_from_list(&heap_g.small_bin_head, (free_chunk_header_t *)next_chunk_ptr);
        remove_chunk_from_list(&heap_g.small_unsorted_list_head, (free_chunk_header_t *)next_chunk_ptr);
        next_chunk_ptr = (size_t *)((uint8_t *)next_chunk_ptr + CHUNK_SIZE_WITHOUT_FLAGS(*next_chunk_ptr));
        next_is_free_to_coalesce = ((void *)next_chunk_ptr < chunk_zone_end) && ((*next_chunk_ptr & IN_USE) == 0);
    }
    *chunk = coalesced_chunk_size;
    if ((*chunk & IN_USE) == 0)
    {
        free_chunk_header_t *free_chunk = (free_chunk_header_t *)chunk;
        set_chunk_footer_size(free_chunk);
    }
}

free_chunk_header_t *try_split_chunk(size_t *chunk_ptr, size_t required_size)
{
    if ((CHUNK_SIZE_WITHOUT_FLAGS(*chunk_ptr) - required_size) <= heap_g.tiny_zone_chunk_max_size)
        return NULL;

    // If the remainder of splitting would be big enough to store
    // more small chunks, split the chunk:
    free_chunk_header_t *remaining_chunk = (free_chunk_header_t *)((uint8_t *)chunk_ptr + required_size);
    remaining_chunk->size = CHUNK_SIZE_WITHOUT_FLAGS(*chunk_ptr) - required_size;
    *chunk_ptr = required_size | (*chunk_ptr & IN_USE) | (*chunk_ptr & PREVIOUS_FREE);
    set_chunk_footer_size(remaining_chunk);

    return remaining_chunk;
}

void remove_zone_from_list(zone_header_t **list_head, zone_header_t *zone)
{
	if (zone == *list_head)
		*list_head = zone->next;
	if (zone->prev != NULL)
		zone->prev->next = zone->next;
	if (zone->next != NULL)
		zone->next->prev = zone->prev;

    zone->prev = NULL;
    zone->next = NULL;
}

void add_chunk_to_list_front(free_chunk_header_t **list_head, free_chunk_header_t *chunk)
{
    if (*list_head != NULL)
	    (*list_head)->prev = chunk;
	chunk->next = *list_head;
	chunk->prev = NULL;
	*list_head = chunk;
}

void remove_chunk_from_list(free_chunk_header_t **list_head, free_chunk_header_t *chunk)
{
    if (*list_head == chunk)
        *list_head = chunk->next;
    if (chunk->prev != NULL)
        chunk->prev->next = chunk->next;
    if (chunk->next != NULL)
        chunk->next->prev = chunk->prev;

    chunk->prev = NULL;
    chunk->next = NULL;
}

void replace_chunk_in_list(free_chunk_header_t **list_head, free_chunk_header_t *chunk_to_remove, free_chunk_header_t *new_chunk)
{
    if (*list_head == chunk_to_remove)
        *list_head = new_chunk;
    if (chunk_to_remove->next != NULL)
        chunk_to_remove->next->prev = new_chunk;
    if (chunk_to_remove->prev != NULL)
        chunk_to_remove->prev->next = new_chunk;

    new_chunk->prev = chunk_to_remove->prev;
    new_chunk->next = chunk_to_remove->next;

    chunk_to_remove->prev = NULL;
    chunk_to_remove->next = NULL;
}

void set_chunk_footer_size(free_chunk_header_t *chunk)
{
    size_t *footer_size = (size_t *)((uint8_t *)chunk + CHUNK_SIZE_WITHOUT_FLAGS(chunk->size) - SIZE_T_SIZE);
	*footer_size = CHUNK_SIZE_WITHOUT_FLAGS(chunk->size);
}

free_chunk_header_t *get_chunk_list_last(free_chunk_header_t *list)
{
    if (!list)
		return (NULL);
	while (list->next)
		list = list->next;
	return list;
}

zone_header_t *get_zone_list_last(zone_header_t *list)
{
    if (!list)
		return (NULL);
	while (list->next)
		list = list->next;
	return list;
}
