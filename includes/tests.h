#include "heap.h"

#define ERROR_MSG_MAX_LEN	                1024
#define RED					                "\x1B[31m"
#define GREEN				                "\x1B[32m"
#define YELLOW					            "\x1B[33m"
#define BLUE					            "\x1B[34m"
#define MAGENTA					            "\x1B[35m"
#define NO_COLOR			                "\x1B[0m"

#define TINY_ZONE_SIZE                      getpagesize()
#define TINY_ZONE_MAX_CHUNK_SIZE            (TINY_ZONE_SIZE / 128)
#define SMALL_ZONE_SIZE                     (getpagesize() * 800)
#define SMALL_ZONE_MAX_CHUNK_SIZE           (SMALL_ZONE_SIZE / 100)
#define NUMBER_OF_TINY_CHUNKS_PER_ZONE      ((TINY_ZONE_SIZE - ZONE_HEADER_T_SIZE) / TINY_ZONE_MAX_CHUNK_SIZE)
#define MIN_NUMBER_OF_SMALL_CHUNKS_PER_ZONE ((SMALL_ZONE_SIZE - ZONE_HEADER_T_SIZE) / SMALL_ZONE_MAX_CHUNK_SIZE)

#define PRINTF_LINE(format, ...) \
    printf("at %s:%d:\n" format, __FILE__, __LINE__, ##__VA_ARGS__)

#define SNPRINTF_LINE(buff, n, format, ...) \
    snprintf(buff, n, "at %s:%d: " format,  __FILE__, __LINE__, ##__VA_ARGS__)

#define FAIL_TEST(test_name, format, ...) { \
	char reason[ERROR_MSG_MAX_LEN]; \
    SNPRINTF_LINE(reason, ERROR_MSG_MAX_LEN, format, ##__VA_ARGS__); \
    fail_test(test_name, reason); }

#define FAIL_TEST_AT(test_name, file_name, line, format, ...) { \
	char reason[ERROR_MSG_MAX_LEN]; \
    snprintf(reason, ERROR_MSG_MAX_LEN, "at %s:%d: " format, file_name, line, ##__VA_ARGS__); \
    fail_test(test_name, reason); }

#define ASSERT_SIZE_EQ(actual, expected) assert_size_eq(test_name, actual, expected, __FILE__, __LINE__)
#define ASSERT_SIZE_LT(actual, expected) assert_size_lt(test_name, actual, expected, __FILE__, __LINE__)
#define ASSERT_POINTER_EQ(actual, expected) assert_pointer_eq(test_name, actual, expected, __FILE__, __LINE__)
#define ASSERT_POINTER_NE(actual, expected) assert_pointer_ne(test_name, actual, expected, __FILE__, __LINE__)
#define ASSERT_NO_CHUNK_OVERLAP(chunk_1_begin, chunk_2_begin) assert_no_chunk_overlap(test_name, chunk_1_begin, chunk_2_begin, __FILE__, __LINE__)

/* Utils */
void pass_test(char *test_name);
void fail_test(char *test_name, char *reason);
size_t zone_list_len(zone_header_t *list);
size_t free_chunk_list_len(free_chunk_header_t *list);
bool overlaps(void *chunk_1_begin, void *chunk_2_begin);

void assert_size_eq(char *test_name, size_t actual, size_t expected, char *file_name, int line_n);
void assert_size_lt(char *test_name, size_t actual, size_t expected, char *file_name, int line_n);
void assert_pointer_eq(char *test_name, void *actual, void *expected, char *file_name, int line_n);
void assert_pointer_ne(char *test_name, void *actual, void *expected, char *file_name, int line_n);
void assert_no_chunk_overlap(char *test_name, size_t *chunk_1_begin, size_t *chunk_2_begin, char *file_name, int line_n);

void hexdump(void *mem, unsigned int len);
void visualise_memory(void);
