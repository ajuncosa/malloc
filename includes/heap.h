#ifndef HEAP_H
#define HEAP_H

#include <stdbool.h>
#include <unistd.h>
#include "alignment.h"

#define IN_USE                          0x01 // whether the current chunk is in use
#define PREVIOUS_FREE                   0x02 // whether the previous chunk is available for coalescing
//#define ARENA                         0x04

//#define MAX_LARGE_CHUNKS			    65536 // maximum number of chunks to allocate with mmap (M_MMAP_MAX)
#define SIZE_T_SIZE	                    ALIGN(sizeof(size_t)) // in-use chunk header metadata size
#define MIN_FREE_CHUNK_SIZE 		    ALIGN(sizeof(free_chunk_header_t)) // free chunk header metadata size
#define ZONE_HEADER_T_SIZE	            ALIGN(sizeof(zone_header_t)) // zone header metadata size
#define CHUNK_SIZE_WITHOUT_FLAGS(size)  (size & ~7u)

typedef struct free_chunk_header_s
{
    size_t size;
    struct free_chunk_header_s *next;
    struct free_chunk_header_s *prev;
} free_chunk_header_t;

typedef struct zone_header_s
{
    struct zone_header_s    *next;
    struct zone_header_s    *prev;
} zone_header_t;

typedef struct heap_s
{
    unsigned int        tiny_zone_size;
    unsigned int        small_zone_size;
    unsigned int        tiny_zone_chunk_max_size;
    unsigned int        small_zone_chunk_max_size;

    zone_header_t       *tiny_zones_head; // doubly linked list of tiny zones
    zone_header_t       *small_zones_head; // doubly linked list of small zones
    zone_header_t       *large_zones_head; // large chunks are in a zone of their own

    free_chunk_header_t *tiny_bin_head;
    free_chunk_header_t *small_bin_head;
    free_chunk_header_t *small_unsorted_list_head;
} heap_t;


bool init_heap(void);
zone_header_t *allocate_new_tiny_zone(void);
zone_header_t *allocate_new_small_zone(void);
// TODO: add a way of freeing tiny zones
void free_small_zone(zone_header_t *ptr_to_zone);
zone_header_t *get_small_zone(void *chunk_ptr);

void *allocate_large_chunk(size_t chunk_size);
void *allocate_small_chunk(size_t chunk_size);
void *allocate_tiny_chunk(void);
void free_large_chunk(size_t *ptr_to_chunk);
void free_small_chunk(size_t *ptr_to_chunk);
void free_tiny_chunk(size_t *ptr_to_chunk);
void *realloc_large_chunk(void *ptr_to_data, size_t *ptr_to_chunk, size_t new_alloc_size);
void *realloc_small_chunk(void *ptr_to_data, size_t *ptr_to_chunk, size_t new_alloc_size, size_t new_chunk_size);
void *realloc_tiny_chunk(void *ptr_to_data, size_t *ptr_to_chunk, size_t new_alloc_size, size_t new_chunk_size);

free_chunk_header_t *coalesce(free_chunk_header_t *chunk);
free_chunk_header_t *try_split_chunk(size_t *chunk_ptr, size_t required_size);

//utils
void remove_zone_from_list(zone_header_t **list_head, zone_header_t *zone);
void add_chunk_to_list_front(free_chunk_header_t **list_head, free_chunk_header_t *chunk);
void add_chunk_to_small_bin(free_chunk_header_t *chunk);
void remove_chunk_from_list(free_chunk_header_t **list_head, free_chunk_header_t *chunk);
void replace_chunk_in_list(free_chunk_header_t **list_head, free_chunk_header_t *chunk_to_remove, free_chunk_header_t *new_chunk);
void set_chunk_footer_size(free_chunk_header_t *chunk);
free_chunk_header_t *get_chunk_list_last(free_chunk_header_t *list);
zone_header_t *get_zone_list_last(zone_header_t *list);

// global
extern heap_t heap_g;

#endif
