#ifndef HEAP_H
#define HEAP_H

#include <stdbool.h>
#include <unistd.h>
#include "alignment.h"

#define IN_USE                      0x01 // whether the current chunk is in use
#define PREVIOUS_FREE               0x02 // whether the previous chunk is available for coalescing
//#define ARENA                       0x04

#define TINY_ZONE_SIZE              4096 //FIXME: this + zone header > page size
#define TINY_ZONE_CHUNK_MAX_SIZE    32 // FIXME: also have in mind the size of the "size" member

#define SMALL_ZONE_SIZE				13107200
// All chunks larger than this value are allocated outside the normal heap,
// using the mmap system call. This way it is guaranteed that the memory for
// these chunks can be returned to the system on free.
#define SMALL_ZONE_CHUNK_MAX_SIZE	131072 // 128 * 1024 (M_MMAP_THRESHOLD) 128kb up to 32mb

#define MAX_LARGE_CHUNKS			65536 // maximum number of chunks to allocate with mmap (M_MMAP_MAX)

#define MIN_FREE_CHUNK_SIZE 		ALIGN(sizeof(free_chunk_header_t))

#define ZONE_HEADER_T_SIZE	        ALIGN(sizeof(zone_header_t)) // zone header metadata size

#define CHUNK_SIZE_WITHOUT_FLAGS(size)  (size & ~7u)

typedef struct free_chunk_header_s
{
    size_t size;
    struct free_chunk_header_s *next;
    struct free_chunk_header_s *prev;
} free_chunk_header_t;

typedef struct zone_header_s
{
    struct zone_header_s		*next;
    struct zone_header_s		*prev;
} zone_header_t;

// TODO: how to know when to unmap tiny zone if empty?
typedef struct heap_s
{
    zone_header_t       *tiny_zones_head; // doubly linked list of tiny zones
    zone_header_t       *small_zones_head; // doubly linked list of small zones
    zone_header_t       *large_zones_head; // large chunks are in a zone of their own

    free_chunk_header_t *tiny_bin_head;
    free_chunk_header_t *small_bin_head;
    free_chunk_header_t *unsorted_small_list_head;
} heap_t;


bool init_heap(void);
zone_header_t *allocate_new_tiny_zone();
zone_header_t *allocate_new_small_zone();
void free_small_zone(zone_header_t *ptr_to_zone);
zone_header_t *get_small_zone(void *chunk_ptr);

void *allocate_large_chunk(size_t chunk_size);
void *allocate_small_chunk(size_t chunk_size);
void *allocate_tiny_chunk(void);
void free_large_chunk(size_t *ptr_to_chunk);
void free_small_chunk(size_t *ptr_to_chunk);
void free_tiny_chunk(size_t *ptr_to_chunk);

void move_chunk_from_unsorted_to_small_bin(free_chunk_header_t *chunk);
free_chunk_header_t *coalesce(free_chunk_header_t *chunk);

//utils
void remove_zone_from_list(zone_header_t **list_head, zone_header_t *zone);
void add_chunk_to_list_front(free_chunk_header_t **list_head, free_chunk_header_t *chunk);
void remove_chunk_from_list(free_chunk_header_t **list_head, free_chunk_header_t *chunk);
void replace_chunk_in_list(free_chunk_header_t **list_head, free_chunk_header_t *chunk_to_remove, free_chunk_header_t *new_chunk);
void set_chunk_footer_size(free_chunk_header_t *chunk);

// global
extern heap_t heap_g;

#endif
