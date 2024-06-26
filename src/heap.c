#include <sys/mman.h>
#include <stdlib.h>
#include <errno.h>
#include "heap.h"
#include "libft.h"

#include <stdio.h> // FIXME: remove

// global
heap_t heap_g = { .tiny_zones_head = NULL,
                  .tiny_bin_head = NULL,
                  .small_zones_head = NULL,
                  .small_bin_head = NULL,
                  .small_unsorted_list_head = NULL,
                  .large_zones_head = NULL };

bool init_heap(void)
{
    heap_g.tiny_zone_size = getpagesize();
    heap_g.small_zone_size = getpagesize() * 800;
    heap_g.tiny_zone_chunk_max_size = heap_g.tiny_zone_size / 128;
    heap_g.small_zone_chunk_max_size = heap_g.small_zone_size / 100;
    printf("TINY ZONE SIZE: %d, SMALL ZONE SIZE: %d, TINY CHUNK MAX: %d, SMALL CHUNK MAX: %d\n", heap_g.tiny_zone_size, heap_g.small_zone_size, heap_g.tiny_zone_chunk_max_size, heap_g.small_zone_chunk_max_size);
    printf("USABLE TINY ZONE SIZE: %lu, USABLE SMALL ZONE SIZE: %lu\n", heap_g.tiny_zone_size - ZONE_HEADER_T_SIZE, heap_g.small_zone_size - ZONE_HEADER_T_SIZE);
    if (heap_g.tiny_zone_chunk_max_size < MIN_FREE_CHUNK_SIZE) // just in case
        return false;
    return true;
}

zone_header_t *allocate_new_tiny_zone(void)
{
    printf ("ALLOCATING NEW TINY ZONE\n");
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
    printf ("ALLOCATING NEW SMALL ZONE\n");
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
	if (munmap(ptr_to_zone, heap_g.small_zone_size) == -1)
	{
		printf("Error: munmap failed with errno: %d\n", errno);
		exit(1);
	}
}

zone_header_t *get_small_zone(void *chunk_ptr)
{
    for (zone_header_t *zone = heap_g.small_zones_head; zone != NULL; zone = zone->next)
    {
        if (chunk_ptr > (void *)zone && chunk_ptr < (void *)((uint8_t *)zone + heap_g.small_zone_size))
        {
            return zone;
        }
    }
    return NULL;
}

void *allocate_large_chunk(size_t chunk_size)
{
	printf("allocating LARGE chunk of size %zu.\n", chunk_size);
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
	printf("allocating small chunk of size %zu.\n", chunk_size);
	//printf("STATE OF THE SMALL LIST0:\n");
    //for (free_chunk_header_t *it = heap_g.small_bin_head; it != NULL; it = it->next)
    //    printf("  size: %zu, ptr: %p\n", it->size, it);

    //printf("STATE OF THE UNSORTED LIST0:\n");
    //for (free_chunk_header_t *it = heap_g.small_unsorted_list_head; it != NULL; it = it->next)
    //    printf("  size: %zu, ptr: %p\n", it->size, it);

    free_chunk_header_t *unsorted_chunk = heap_g.small_unsorted_list_head;
    free_chunk_header_t *next_unsorted_chunk = NULL;
	while (unsorted_chunk != NULL)
	{
		if (CHUNK_SIZE_WITHOUT_FLAGS(unsorted_chunk->size) == chunk_size)
		{
            //printf("Found a match in the unsorted list!\n");
			new_chunk = unsorted_chunk;
            //printf("Removing chunk from unsorted list...\n");
            remove_chunk_from_list(&heap_g.small_unsorted_list_head, new_chunk);
			break;
		}
        //printf("Not a match. Checking if it is possible to coalesce...\n");

        // Deferred coalescing
        unsorted_chunk = coalesce(unsorted_chunk);
        next_unsorted_chunk = unsorted_chunk->next;
        //printf("NEXT: %p\n", next_unsorted_chunk);
        
        if (CHUNK_SIZE_WITHOUT_FLAGS(unsorted_chunk->size) == chunk_size)
        {
            //printf("The coalesced chunk is a match!\n");
            new_chunk = unsorted_chunk;
            //printf("Removing chunk from unsorted list...\n");
            remove_chunk_from_list(&heap_g.small_unsorted_list_head, new_chunk);
            break;
        }
        // If the zone is empty and there are smaller chunks available, free the zone
        // (otherwise, we will need to immediately allocate a new chunk after this anyway)
        if (CHUNK_SIZE_WITHOUT_FLAGS(unsorted_chunk->size) == (heap_g.small_zone_size - ZONE_HEADER_T_SIZE)
            && heap_g.small_bin_head != NULL
            && unsorted_chunk->size > heap_g.small_bin_head->size) // FIXME: is this check ok?
        {
            printf("Zone is empty, freeing...\n");
            zone_header_t *zone = get_small_zone(unsorted_chunk);
            remove_chunk_from_list(&heap_g.small_unsorted_list_head, unsorted_chunk);
            free_small_zone(zone);
        }
        else
        {
            remove_chunk_from_list(&heap_g.small_unsorted_list_head, unsorted_chunk);
            add_chunk_to_small_bin(unsorted_chunk);
        }
        //printf("STATE OF THE SMALL LIST1:\n");
        //for (free_chunk_header_t *it = heap_g.small_bin_head; it != NULL; it = it->next)
        //    printf("  size: %zu, ptr: %p\n", it->size, it);
//
        //printf("STATE OF THE UNSORTED LIST1:\n");
        //for (free_chunk_header_t *it = heap_g.small_unsorted_list_head; it != NULL; it = it->next)
        //    printf("  size: %zu, ptr: %p\n", it->size, it);
            
        unsorted_chunk = next_unsorted_chunk;
	}
    if (new_chunk == NULL)
    {
        //printf("Could not find a match in the unsorted list; checking the small bin\n");
	    for (free_chunk_header_t *chunk = heap_g.small_bin_head; chunk != NULL; chunk = chunk->next)
        {
            if (CHUNK_SIZE_WITHOUT_FLAGS(chunk->size) >= chunk_size)
            {
                //printf("Found a match in the small bin\n");
                new_chunk = chunk;
                break;
            }
        }
        if (new_chunk ==  NULL)
        {
            // the zone is full or there is not a big enough free chunk
            //printf("no big enough free chunks in small bin: need a new small zone\n");
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
            zone_header_t *new_chunk_zone = get_small_zone(new_chunk);
            size_t *next_chunk = (size_t *)((uint8_t *)new_chunk + CHUNK_SIZE_WITHOUT_FLAGS(new_chunk->size));
            if (next_chunk != (size_t *)((uint8_t *)new_chunk_zone + heap_g.small_zone_size))
            {
                //printf("Unsetting next chunk's PREVIOUS_FREE\n");
                *next_chunk &= ~PREVIOUS_FREE;
            }
            else
            {
                //printf("NOT unsetting next chunk's PREVIOUS_FREE bc next_chunk is end of zone\n");
            }
        }
    }

	new_chunk->prev = NULL;
	new_chunk->next = NULL;
	new_chunk->size |= IN_USE;

	return (uint8_t *)new_chunk + SIZE_T_SIZE;
}

void *allocate_tiny_chunk(void)
{
	printf("allocating tiny chunk\n");
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
    remove_zone_from_list(&heap_g.large_zones_head, ptr_to_zone);
	if (munmap(ptr_to_zone, ALIGN(*ptr_to_chunk) + ZONE_HEADER_T_SIZE) == -1)
	{
		printf("Error: munmap failed with errno: %d\n", errno);
		exit(1);
	}
}

void free_small_chunk(size_t *ptr_to_chunk)
{
	free_chunk_header_t *freed_chunk = (free_chunk_header_t *)ptr_to_chunk;
	zone_header_t *freed_chunk_zone = get_small_zone(freed_chunk);
	size_t *next_chunk = (size_t *)((uint8_t *)freed_chunk + CHUNK_SIZE_WITHOUT_FLAGS(freed_chunk->size));
	if (next_chunk != (size_t *)((uint8_t *)freed_chunk_zone + heap_g.small_zone_size))
	{
		//printf("Setting next chunk to PREVIOUS_FREE.\n");
		*next_chunk |= PREVIOUS_FREE;
	}
	else
	{
		//printf("NOT setting next chunk's PREVIOUS_FREE bc next_chunk is end of zone\n");
	}

    add_chunk_to_list_front(&heap_g.small_unsorted_list_head, freed_chunk);
	freed_chunk->size &= ~IN_USE;
    set_chunk_footer_size(freed_chunk);
}

void free_tiny_chunk(size_t *ptr_to_chunk)
{
	free_chunk_header_t *freed_chunk = (free_chunk_header_t *)ptr_to_chunk;
    add_chunk_to_list_front(&heap_g.tiny_bin_head, freed_chunk);
	freed_chunk->size &= ~IN_USE;
}

void *realloc_large_chunk(void *ptr_to_data, size_t *ptr_to_chunk, size_t new_alloc_size)
{
	void *new_ptr = NULL;
	size_t copy_size = new_alloc_size < CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) ? new_alloc_size : CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk);

    if ((new_ptr = malloc(new_alloc_size)) == NULL)
        return NULL;

    ft_memcpy(new_ptr, ptr_to_data, copy_size);
    free_large_chunk(ptr_to_chunk);

    return new_ptr;
}

void *realloc_small_chunk(void *ptr_to_data, size_t *ptr_to_chunk, size_t new_alloc_size, size_t new_chunk_size)
{
	void *new_ptr = NULL;
	size_t copy_size = new_alloc_size < CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) ? new_alloc_size : CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk);
	
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
            add_chunk_to_small_bin(remaining_chunk);
            *ptr_to_chunk |= IN_USE;
        }
        return ptr_to_data;
    }
    // The allocation is growing and the chunk resulting from the realloc will be "small"
    else
    {
        // TODO:
        //free_chunk_header_t *coalesced = coalesce(ptr_to_chunk);
        free_chunk_header_t *coalesced = (free_chunk_header_t*)ptr_to_chunk;
        // If the coalesced chunk is still too small, do a malloc-copy-free:
        if (coalesced->size < new_chunk_size)
        {
            if ((new_ptr = malloc(new_alloc_size)) == NULL)
                return NULL;

            ft_memcpy(new_ptr, ptr_to_data, copy_size);
            free_small_chunk(ptr_to_chunk);
        }
    }
    return new_ptr;
}

void *realloc_tiny_chunk(void *ptr_to_data, size_t *ptr_to_chunk, size_t new_alloc_size, size_t new_chunk_size)
{
	void *new_ptr = NULL;
	size_t copy_size = new_alloc_size < CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk) ? new_alloc_size : CHUNK_SIZE_WITHOUT_FLAGS(*ptr_to_chunk);

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

free_chunk_header_t *coalesce(free_chunk_header_t *chunk)
{
    free_chunk_header_t *coalesced_chunk = chunk;

    void *chunk_zone_begin = (uint8_t *)get_small_zone(chunk) + ZONE_HEADER_T_SIZE;
    void *chunk_zone_end = (void *)((uint8_t *)chunk_zone_begin + heap_g.small_zone_size - ZONE_HEADER_T_SIZE);

    // If PREVIOUS_FREE is not set, the previous chunk footer size_t is not usable (those bytes might be in use by the user payload)
    bool prev_is_free_to_coalesce = ((void *)chunk > chunk_zone_begin) && ((chunk->size & PREVIOUS_FREE) == PREVIOUS_FREE);
    size_t *next_chunk_ptr = (size_t *)((uint8_t *)chunk + CHUNK_SIZE_WITHOUT_FLAGS(chunk->size));
    bool next_is_free_to_coalesce = ((void *)next_chunk_ptr < chunk_zone_end) && ((*next_chunk_ptr & IN_USE) == 0);

    if (prev_is_free_to_coalesce && !next_is_free_to_coalesce)
    {
        size_t prev_size = *(size_t *)((uint8_t *)chunk - SIZE_T_SIZE);
        size_t *prev_chunk_ptr = (size_t *)((uint8_t *)chunk - prev_size);
        /*
        while (prev_is_free_to_coalesce == true)
        {
            prev_size = *(size_t *)((uint8_t *)chunk - SIZE_T_SIZE);
            prev_chunk_ptr = (size_t *)((uint8_t *)chunk - prev_size);
            prev_is_free_to_coalesce = ((void *)chunk > chunk_zone_begin) && ((chunk->size & PREVIOUS_FREE) == PREVIOUS_FREE);
        }
        */
        if ((*prev_chunk_ptr & IN_USE) == 0)
        {
            printf("Colaescing with prev chunk of size: %zu.\n", prev_size);
            free_chunk_header_t *prev_free_chunk = (free_chunk_header_t *)prev_chunk_ptr;
            // FIXME: will these never have previous_free? do I need to do a loop?
            prev_free_chunk->size += CHUNK_SIZE_WITHOUT_FLAGS(chunk->size);
            set_chunk_footer_size(prev_free_chunk);
            coalesced_chunk = prev_free_chunk;

            remove_chunk_from_list(&heap_g.small_bin_head, coalesced_chunk);
            remove_chunk_from_list(&heap_g.small_unsorted_list_head, coalesced_chunk);

            replace_chunk_in_list(&heap_g.small_unsorted_list_head, chunk, coalesced_chunk);
        }
        else
        {
            printf("Something does not seem ok when coalescing: PREVIOUS_FREE is set, but the previous chunk's IN_USE is also set.");
            exit(1);
        }
    }
    else if (!prev_is_free_to_coalesce && next_is_free_to_coalesce)
    {
        free_chunk_header_t *next_free_chunk = (free_chunk_header_t *)next_chunk_ptr;
        printf("Colaescing with next chunk of size: %zu.\n", CHUNK_SIZE_WITHOUT_FLAGS(*next_chunk_ptr));
        // FIXME: will these never have previous_free? do I need to do a loop?
        chunk->size += CHUNK_SIZE_WITHOUT_FLAGS(next_free_chunk->size);
        set_chunk_footer_size(chunk);
        coalesced_chunk = chunk;

        remove_chunk_from_list(&heap_g.small_bin_head, next_free_chunk);
        remove_chunk_from_list(&heap_g.small_unsorted_list_head, next_free_chunk);
    }
    else if (prev_is_free_to_coalesce && next_is_free_to_coalesce)
    {
        size_t prev_size = *(size_t *)((uint8_t *)chunk - SIZE_T_SIZE);
        size_t *prev_chunk_ptr = (size_t *)((uint8_t *)chunk - prev_size);
        if ((*prev_chunk_ptr & IN_USE) == 0)
        {
            printf("Colaescing with prev and next chunk of sizes: %zu, %zu.\n", *(size_t *)((uint8_t *)chunk - SIZE_T_SIZE), CHUNK_SIZE_WITHOUT_FLAGS(*next_chunk_ptr));
            free_chunk_header_t *prev_free_chunk = (free_chunk_header_t *)prev_chunk_ptr;
            free_chunk_header_t *next_free_chunk = (free_chunk_header_t *)next_chunk_ptr;
            // FIXME: will these never have previous_free? do I need to do a loop?
            prev_free_chunk->size += + CHUNK_SIZE_WITHOUT_FLAGS(chunk->size) + CHUNK_SIZE_WITHOUT_FLAGS(next_free_chunk->size);
            set_chunk_footer_size(prev_free_chunk);
            coalesced_chunk = prev_free_chunk;

            remove_chunk_from_list(&heap_g.small_bin_head, coalesced_chunk);
            remove_chunk_from_list(&heap_g.small_unsorted_list_head, coalesced_chunk);

            remove_chunk_from_list(&heap_g.small_bin_head, next_free_chunk);
            remove_chunk_from_list(&heap_g.small_unsorted_list_head, next_free_chunk);

            replace_chunk_in_list(&heap_g.small_unsorted_list_head, chunk, coalesced_chunk);
        }
        else
        {
            printf("Something does not seem ok when coalescing: PREVIOUS_FREE is set, but the previous chunk's IN_USE is also set.");
            exit(1);
        }
    }
    printf("coalesced chunk size: %zu\n", CHUNK_SIZE_WITHOUT_FLAGS(coalesced_chunk->size));
    return coalesced_chunk;
}

free_chunk_header_t *try_split_chunk(size_t *chunk_ptr, size_t required_size)
{
    if ((CHUNK_SIZE_WITHOUT_FLAGS(*chunk_ptr) - required_size) <= heap_g.tiny_zone_chunk_max_size)
        return NULL;

    //printf("Splitting chunk.\n");

    // If the remainder of splitting would be big enough to store
    // more small chunks, split the chunk:
    free_chunk_header_t *remaining_chunk = (free_chunk_header_t *)((uint8_t *)chunk_ptr + required_size);
    remaining_chunk->size = CHUNK_SIZE_WITHOUT_FLAGS(*chunk_ptr) - required_size;
    *chunk_ptr = required_size;
    set_chunk_footer_size(remaining_chunk);

    //printf("Remaining free chunk size: %zu.\n", remaining_chunk->size);
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

