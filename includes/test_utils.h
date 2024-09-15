#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include "heap.h"

#define ERROR_MSG_MAX_LEN                   1024
#define RED                                 "\x1B[31m"
#define GREEN                               "\x1B[32m"
#define YELLOW                              "\x1B[33m"
#define BLUE                                "\x1B[34m"
#define MAGENTA                             "\x1B[35m"
#define NO_COLOR                            "\x1B[0m"

#define TINY_ZONE_SIZE                      PAGE_SIZE
#define TINY_ZONE_MAX_CHUNK_SIZE            (TINY_ZONE_SIZE / 128)
#define SMALL_ZONE_SIZE                     (PAGE_SIZE * 800)
#define SMALL_ZONE_MAX_CHUNK_SIZE           (SMALL_ZONE_SIZE / 100)
#define NUMBER_OF_TINY_CHUNKS_PER_ZONE      ((TINY_ZONE_SIZE - ZONE_HEADER_T_SIZE) / TINY_ZONE_MAX_CHUNK_SIZE)
#define MIN_NUMBER_OF_SMALL_CHUNKS_PER_ZONE ((SMALL_ZONE_SIZE - ZONE_HEADER_T_SIZE) / SMALL_ZONE_MAX_CHUNK_SIZE)

#define PRINT_FAIL_TEST_AT(file_name, line) {   \
    print_str(RED "at ");                       \
    print_str(file_name);                       \
    print_str(":");                             \
    print_size(line);                           \
    print_str(NO_COLOR); }

#define ASSERT_CHAR_EQ(actual, expected) assert_char_eq(actual, expected, __FILE__, __LINE__)
#define ASSERT_SIZE_EQ(actual, expected) assert_size_eq(actual, expected, __FILE__, __LINE__)
#define ASSERT_SIZE_LT(actual, expected) assert_size_lt(actual, expected, __FILE__, __LINE__)
#define ASSERT_POINTER_EQ(actual, expected) assert_pointer_eq(actual, expected, __FILE__, __LINE__)
#define ASSERT_POINTER_NE(actual, expected) assert_pointer_ne(actual, expected, __FILE__, __LINE__)
#define ASSERT_NO_CHUNK_OVERLAP(chunk_1_begin, chunk_2_begin) assert_no_chunk_overlap(chunk_1_begin, chunk_2_begin, __FILE__, __LINE__)
#define ASSERT_ALIGNMENT(ptr) assert_alignment(ptr, __FILE__, __LINE__)

#define RANDOM_IN_RANGE(min, max) (rand() % (max + 1 - min) + min)

/* Utils */
size_t zone_list_len(zone_header_t *list);
size_t free_chunk_list_len(free_chunk_header_t *list);
bool overlaps(void *chunk_1_begin, void *chunk_2_begin);

void assert_char_eq(char actual, char expected, char *file_name, int line_n);
void assert_size_eq(size_t actual, size_t expected, char *file_name, int line_n);
void assert_size_lt(size_t actual, size_t expected, char *file_name, int line_n);
void assert_pointer_eq(void *actual, void *expected, char *file_name, int line_n);
void assert_pointer_ne(void *actual, void *expected, char *file_name, int line_n);
void assert_no_chunk_overlap(size_t *chunk_1_begin, size_t *chunk_2_begin, char *file_name, int line_n);
void assert_alignment(void *ptr, char *file_name, int line_n);

void visualise_memory(void);

#endif