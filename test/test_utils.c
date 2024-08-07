#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "test_utils.h"

void pass_test(char *test_name)
{
	printf(GREEN "[TEST OK] %s\n" NO_COLOR, test_name);
}

void fail_test(char *test_name, char *reason)
{
	printf(RED "[TEST KO] %s: %s\n" NO_COLOR, test_name, reason);
	freopen("test_output.ansi", "w", stdout);
	visualise_memory();
	fclose(stdout);
	freopen("/dev/tty", "w", stdout);
	exit(1);
}

void assert_char_eq(char *test_name, char actual, char expected, char *file_name, int line_n)
{
	if (actual != expected)
		FAIL_TEST_AT(test_name, file_name, line_n, "Char equality assertion failed (expected: %c, actual: %c)", expected, actual);
}

void assert_size_eq(char *test_name, size_t actual, size_t expected, char *file_name, int line_n)
{
	if (actual != expected)
		FAIL_TEST_AT(test_name, file_name, line_n, "Size equality assertion failed (expected: %zu, actual: %zu)", expected, actual);
}

void assert_size_lt(char *test_name, size_t actual, size_t expected, char *file_name, int line_n)
{
	if (actual >= expected)
		FAIL_TEST_AT(test_name, file_name, line_n, "Size inequality assertion failed (expected: <%zu, actual: %zu)", expected, actual);
}

void assert_pointer_eq(char *test_name, void *actual, void *expected, char *file_name, int line_n)
{
	if (actual != expected)
		FAIL_TEST_AT(test_name, file_name, line_n, "Pointer equality assertion failed (expected: %p, actual: %p)", expected, actual);
}

void assert_pointer_ne(char *test_name, void *actual, void *expected, char *file_name, int line_n)
{
	if (actual == expected)
		FAIL_TEST_AT(test_name, file_name, line_n, "Pointer inequality assertion failed: %p and %p are equal", expected, actual);
}

void assert_no_chunk_overlap(char *test_name, size_t *chunk_1_begin, size_t *chunk_2_begin, char *file_name, int line_n)
{
	if (overlaps(chunk_1_begin, chunk_2_begin))
	{
		size_t *chunk_1_end = (size_t *)((char *)chunk_1_begin + CHUNK_SIZE_WITHOUT_FLAGS(*chunk_1_begin));
		size_t *chunk_2_end = (size_t *)((char *)chunk_2_begin + CHUNK_SIZE_WITHOUT_FLAGS(*chunk_2_begin));
		FAIL_TEST_AT(test_name, file_name, line_n, "Memory at [%p - %p] and [%p - %p] is overlapping", chunk_1_begin, chunk_1_end, chunk_2_begin, chunk_2_end);
	}
}

size_t zone_list_len(zone_header_t *list)
{
	size_t len = 0;
	for (zone_header_t *it = list; it != NULL; it = it->next)
		len++;
	return len;
}

size_t free_chunk_list_len(free_chunk_header_t *list)
{
	size_t len = 0;
	for (free_chunk_header_t *it = list; it != NULL; it = it->next)
		len++;
	return len;
}

bool overlaps(void *ptr1, void *ptr2)
{
	if (ptr1 == NULL || ptr2 == NULL)
		return false;

	size_t *chunk1_begin = (size_t *)ptr1;
	size_t *chunk2_begin = (size_t *)ptr2;
	size_t chunk1_size = CHUNK_SIZE_WITHOUT_FLAGS(*chunk1_begin);
	size_t chunk2_size = CHUNK_SIZE_WITHOUT_FLAGS(*chunk2_begin);
	size_t *chunk1_end = (size_t *)((char *)chunk1_begin + (chunk1_size));
	size_t *chunk2_end = (size_t *)((char *)chunk2_begin + chunk2_size);
	if (chunk1_begin < chunk2_begin)
	{
		if (chunk1_end > chunk2_begin)
			return true;
	}
	else if (chunk1_begin > chunk2_begin)
	{
		if (chunk2_end > chunk1_begin)
			return true;
	}
	else
		return true;

	return false;
}

void hexdump(void *mem, unsigned int len)
{
    unsigned int i, j, k;
    
    for(i = 0; i < len + ((len % 16) ? (16 - len % 16) : 0); i++) {
        if(i % 16 == 0) {
            printf("0x%06x: ", i);
            k = 0;
        }

        if (k == 4 || k == 12)
            printf(" ");

        if (k == 8)
            printf("   ");
        k++;

        if(i < len) {
            printf("%02x ", 0xFF & ((char*)mem)[i]);
        }
        else {
            printf("   ");
        }
        
        if(i % 16 == (16 - 1)) {
            for(j = i - (16 - 1); j <= i; j++) {
                if(j >= len) {
                    putchar(' ');
                }
                else if(isprint(((char*)mem)[j])) {
                    putchar(0xFF & ((char*)mem)[j]);        
                }
                else {
                    putchar('.');
                }
            }
            putchar('\n');
        }
    }
}

void visualise_memory(void)
{
	for (zone_header_t *tiny_zone = heap_g.tiny_zones_head; tiny_zone != NULL; tiny_zone = tiny_zone->next)
	{
		printf("| %p " GREEN "tiny zone header (%zu bytes): %p %p" NO_COLOR " |\n",
			tiny_zone,
			ZONE_HEADER_T_SIZE,
			tiny_zone->prev,
			tiny_zone->next);

		size_t *chunk_ptr = (size_t *)((uint8_t *)tiny_zone + ZONE_HEADER_T_SIZE);
		size_t chunk_size = CHUNK_SIZE_WITHOUT_FLAGS(*chunk_ptr);
		for (size_t i = 0; i < ((heap_g.tiny_zone_size - ZONE_HEADER_T_SIZE) / heap_g.tiny_zone_chunk_max_size); i++)
    	{
			if ((*chunk_ptr & IN_USE) == IN_USE)
			{
				printf("| %p " BLUE "chunk header (%zu bytes): %zu %d%d" NO_COLOR " | " MAGENTA "in use body (%zu bytes)" NO_COLOR " |\n" ,
					chunk_ptr, SIZE_T_SIZE, chunk_size, (uint8_t)(*chunk_ptr & PREVIOUS_FREE), (uint8_t)(*chunk_ptr & IN_USE),
					chunk_size - SIZE_T_SIZE);
			}
			else
			{
				free_chunk_header_t *free_chunk_ptr = (free_chunk_header_t *)chunk_ptr;
    			size_t *footer_size = (size_t *)((uint8_t *)chunk_ptr + chunk_size - SIZE_T_SIZE);
				printf("| %p " BLUE "chunk header (%zu bytes): %zu %d%d %p %p" NO_COLOR " | " YELLOW "free bytes (%zu bytes)" NO_COLOR " | " BLUE "size (%zu bytes): %zu" NO_COLOR " |\n" ,
					chunk_ptr, sizeof(free_chunk_header_t), chunk_size, (uint8_t)(*chunk_ptr & PREVIOUS_FREE), (uint8_t)(*chunk_ptr & IN_USE), free_chunk_ptr->prev, free_chunk_ptr->next,
					chunk_size - sizeof(free_chunk_header_t) - SIZE_T_SIZE, SIZE_T_SIZE, *footer_size);
			}
        	chunk_ptr = (size_t *)((uint8_t *)(chunk_ptr) + chunk_size);
		}
	}
	for (zone_header_t *small_zone = heap_g.small_zones_head; small_zone != NULL; small_zone = small_zone->next)
	{
		printf("| %p " GREEN "small zone header (%zu bytes): %p %p" NO_COLOR " |\n",
			small_zone,
			ZONE_HEADER_T_SIZE,
			small_zone->prev,
			small_zone->next);

		size_t *chunk_ptr = (size_t *)((uint8_t *)small_zone + ZONE_HEADER_T_SIZE);
		size_t chunk_size;
		for (size_t i = 0; i < (heap_g.small_zone_size - ZONE_HEADER_T_SIZE); i += chunk_size)
    	{
			chunk_size = CHUNK_SIZE_WITHOUT_FLAGS(*chunk_ptr);
			uint8_t in_use_bit_flag = (uint8_t)(*chunk_ptr & IN_USE);
			uint8_t prev_free_bit_flag = (uint8_t)(*chunk_ptr & PREVIOUS_FREE);
			if ((*chunk_ptr & IN_USE) == IN_USE)
			{
				size_t in_use_bytes = chunk_size - SIZE_T_SIZE;
				printf("| %p " BLUE "chunk header (%zu bytes): %zu %d%d" NO_COLOR " | " MAGENTA "in use body (%zu bytes)" NO_COLOR " |\n" ,
					chunk_ptr, SIZE_T_SIZE, chunk_size, prev_free_bit_flag, in_use_bit_flag, // chunk header
					in_use_bytes); // chunk data
			}
			else
			{
				free_chunk_header_t *free_chunk_ptr = (free_chunk_header_t *)chunk_ptr;
    			size_t *footer_size = (size_t *)((uint8_t *)chunk_ptr + chunk_size - SIZE_T_SIZE);
				size_t free_bytes = chunk_size - sizeof(free_chunk_header_t) - SIZE_T_SIZE;
				printf("| %p " BLUE "chunk header (%zu bytes): %zu %d%d %p %p" NO_COLOR " | " YELLOW "free bytes (%zu bytes)" NO_COLOR " | " BLUE "size (%zu bytes): %zu" NO_COLOR " |\n" ,
					chunk_ptr, sizeof(free_chunk_header_t), chunk_size, prev_free_bit_flag, in_use_bit_flag, free_chunk_ptr->prev, free_chunk_ptr->next, // chunk header
					free_bytes, // free space
					SIZE_T_SIZE, *footer_size); // footer size
			}
        	chunk_ptr = (size_t *)((uint8_t *)(chunk_ptr) + chunk_size);
		}
	}  

	for (zone_header_t *large_zone = heap_g.large_zones_head; large_zone != NULL; large_zone = large_zone->next)
	{
		printf("| %p " GREEN "large zone header (%zu bytes): %p %p" NO_COLOR " |\n",
			large_zone,
			ZONE_HEADER_T_SIZE,
			large_zone->prev,
			large_zone->next);
		size_t *chunk_ptr = (size_t *)((uint8_t *)large_zone + ZONE_HEADER_T_SIZE);
		size_t chunk_size = CHUNK_SIZE_WITHOUT_FLAGS(*chunk_ptr);
		uint8_t in_use_bit_flag = (uint8_t)(*chunk_ptr & IN_USE);
		uint8_t prev_free_bit_flag = (uint8_t)(*chunk_ptr & PREVIOUS_FREE);
		if ((*chunk_ptr & IN_USE) == IN_USE)
		{
			size_t in_use_bytes = chunk_size - SIZE_T_SIZE;
			printf("| %p " BLUE "chunk header (%zu bytes): %zu %d%d" NO_COLOR " | " MAGENTA "in use body (%zu bytes)" NO_COLOR " |\n" ,
				chunk_ptr, SIZE_T_SIZE, chunk_size, prev_free_bit_flag, in_use_bit_flag, // chunk header
				in_use_bytes); // chunk data
		}
		else // should never go in here
		{
			free_chunk_header_t *free_chunk_ptr = (free_chunk_header_t *)chunk_ptr;
			size_t free_bytes = chunk_size - sizeof(free_chunk_header_t) - SIZE_T_SIZE;
			printf("| %p " BLUE "chunk header (%zu bytes): %zu %d%d %p %p" NO_COLOR " | " YELLOW "free bytes (%zu bytes)" NO_COLOR " |\n" ,
				chunk_ptr, sizeof(free_chunk_header_t), chunk_size, prev_free_bit_flag, in_use_bit_flag, free_chunk_ptr->prev, free_chunk_ptr->next, // chunk header
				free_bytes); // free space
		}
	}
}
