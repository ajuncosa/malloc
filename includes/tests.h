#include "heap.h"

#define ERROR_MSG_MAX_LEN	1024
#define GREEN				"\x1B[32m"
#define RED					"\x1B[31m"
#define NO_COLOR			"\x1B[0m"
#define NUMBER_OF_TINY_CHUNKS_PER_ZONE      ((heap_g.tiny_zone_size - ZONE_HEADER_T_SIZE) / heap_g.tiny_zone_chunk_max_size)
#define MIN_NUMBER_OF_SMALL_CHUNKS_PER_ZONE ((heap_g.small_zone_size - ZONE_HEADER_T_SIZE) / heap_g.small_zone_chunk_max_size)

/* Tests */
void test_init();
void test_tiny_malloc_1(); // malloc(1)
void test_tiny_malloc_2(); // 3 tiny mallocs
void test_tiny_malloc_3(); // NUMBER_OF_TINY_CHUNKS_PER_ZONE tiny mallocs
void test_tiny_malloc_4(); // NUMBER_OF_TINY_CHUNKS_PER_ZONE + 1 tiny mallocs

/* Utils */
void pass_test(char *test_name);
void fail_test(char *test_name, char *reason);
size_t zone_list_len(zone_header_t *list);
size_t free_chunk_list_len(free_chunk_header_t *list);
bool overlaps(void *ptr1, void *ptr2);
