#ifndef HEAP_H
#define HEAP_H

#include <stdbool.h>
#include <unistd.h>
#include "alignment.h"

#define TINY_ZONE_SIZE				4096
#define TINY_ZONE_CHUNK_MAX_SIZE	32

#define SMALL_ZONE_SIZE				13107200
// All chunks larger than this value are allocated outside the normal heap,
// using the mmap system call. This way it is guaranteed that the memory for
// these chunks can be returned to the system on free.
#define SMALL_ZONE_CHUNK_MAX_SIZE	131072 // 128 * 1024 (M_MMAP_THRESHOLD) 128kb up to 32mb

#define MAX_LARGE_CHUNKS			65536 // maximum number of chunks to allocate with mmap (M_MMAP_MAX)

#define MIN_FREE_CHUNK_SIZE 		ALIGN(sizeof(free_chunk_header_t))

typedef struct free_chunk_header_s
{
	size_t size;
	struct free_chunk_header_s *next;
	struct free_chunk_header_s *prev;
} free_chunk_header_t;

typedef struct inuse_chunk_header_s
{
	size_t size; // chunk size (aligned)
	//size_t data_size; // real data size that was requested by the user (not necessarily aligned)
} inuse_chunk_header_t;

// large allocations are in a zone of their own, and never reused
typedef struct mmapped_chunk_header_s
{
	size_t size; // chunk size (aligned)
	struct mmapped_chunk_header_s *next;
	struct mmapped_chunk_header_s *prev;
} mmapped_chunk_header_t;

// TODO: multiple zones
/*
typedef struct zone_s
{
	size_t				remaining_free_bytes; // to know when the zone is full and we need to allocate a new one
	struct zone_s		*next_zone;
	struct zone_s		*prev_zone;
	void				*bin; // doubly linked list of chunks
} zone_t;
*/

typedef struct heap_s
{
    // TODO: make tiny and small linked lists of zones (instead of chunks)
	void 			        *tiny_zones_head; // doubly linked list of tiny zones
	void			        *small_zones_head;
	mmapped_chunk_header_t	*large_zones_head; // large chunks are in a zone of their own
} heap_t;


bool init_heap(void);

// global
extern heap_t heap_g;

#endif
