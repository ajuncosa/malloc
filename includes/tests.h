#include "heap.h"

#define ERROR_MSG_MAX_LEN	                1024
#define GREEN				                "\x1B[32m"
#define RED					                "\x1B[31m"
#define NO_COLOR			                "\x1B[0m"
#define TINY_ZONE_SIZE                      getpagesize()
#define TINY_ZONE_MAX_CHUNK_SIZE            (TINY_ZONE_SIZE / 128)
#define SMALL_ZONE_SIZE                     (getpagesize() * 800)
#define SMALL_ZONE_MAX_CHUNK_SIZE           (SMALL_ZONE_SIZE / 100)
#define NUMBER_OF_TINY_CHUNKS_PER_ZONE      ((TINY_ZONE_SIZE - ZONE_HEADER_T_SIZE) / TINY_ZONE_MAX_CHUNK_SIZE)
#define MIN_NUMBER_OF_SMALL_CHUNKS_PER_ZONE ((SMALL_ZONE_SIZE - ZONE_HEADER_T_SIZE) / SMALL_ZONE_MAX_CHUNK_SIZE)

/* Utils */
void pass_test(char *test_name);
void fail_test(char *test_name, char *reason);
size_t zone_list_len(zone_header_t *list);
size_t free_chunk_list_len(free_chunk_header_t *list);
bool overlaps(void *ptr1, void *ptr2);
