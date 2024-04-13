#ifndef HEAP_H
#define HEAP_H

#include <stdbool.h>
#include <unistd.h>
#include "alignment.h"

//#define PREVIOUS_IN_USE             0x01
#define IN_USE                      0x01
#define MMAPPED                     0x02
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
    //size_t				remaining_free_bytes; // to know when the zone is full and we need to allocate a new one
    struct zone_header_s		*next;
    struct zone_header_s		*prev;
    //void				*bin; // doubly linked list of chunks
} zone_header_t;

typedef struct heap_s
{
    void    *tiny_zones_head; // doubly linked list of tiny zones
    void    *small_zones_head; // doubly linked list of small zones
    void    *large_zones_head; // large chunks are in a zone of their own

    free_chunk_header_t *tiny_bin_head;
    // TODO: create multiple bins for small zones?
    free_chunk_header_t *small_bin_head;
} heap_t;


bool init_heap(void);

// global
extern heap_t heap_g;

#endif
