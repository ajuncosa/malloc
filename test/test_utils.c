#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include "test_utils.h"
#include "utils.h"

void assert_char_eq(char actual, char expected, char *file_name, int line_n)
{
    if (actual != expected)
    {
        PRINT_FAIL_TEST_AT(file_name, line_n);
        print_str(RED ": Char equality assertion failed (expected: ");
        ft_putchar(expected);
        print_str(", actual: ");
        ft_putchar(actual);
        print_str(")" NO_COLOR);
        print_endl();
        exit(1);
    }
}

void assert_size_eq(size_t actual, size_t expected, char *file_name, int line_n)
{
    if (actual != expected)
    {
        PRINT_FAIL_TEST_AT(file_name, line_n);
        print_str(RED ": Size equality assertion failed (expected: ");
        print_size(expected);
        print_str(", actual: ");
        print_size(actual);
        print_str(")" NO_COLOR);
        print_endl();
        exit(1);
    }
}

void assert_size_lt(size_t actual, size_t expected, char *file_name, int line_n)
{
    if (actual >= expected)
    {
        PRINT_FAIL_TEST_AT(file_name, line_n);
        print_str(RED ": Size inequality assertion failed (expected: <");
        print_size(expected);
        print_str(", actual: ");
        print_size(actual);
        print_str(")" NO_COLOR);
        print_endl();
        exit(1);
    }
}

void assert_pointer_eq(void *actual, void *expected, char *file_name, int line_n)
{
    if (actual != expected)
    {
        PRINT_FAIL_TEST_AT(file_name, line_n);
        print_str(RED ": Pointer equality assertion failed (expected: ");
        print_address_hex(expected);
        print_str(", actual: ");
        print_address_hex(actual);
        print_str(")" NO_COLOR);
        print_endl();
        exit(1);
    }
}

void assert_pointer_ne(void *actual, void *expected, char *file_name, int line_n)
{
    if (actual == expected)
    {
        PRINT_FAIL_TEST_AT(file_name, line_n);
        print_str(RED ": Pointer inequality assertion failed: ");
        print_address_hex(expected);
        print_str(" and ");
        print_address_hex(actual);
        print_str(" are equal" NO_COLOR);
        print_endl();
        exit(1);
    }
}

void assert_no_chunk_overlap(size_t *chunk_1_begin, size_t *chunk_2_begin, char *file_name, int line_n)
{
    if (overlaps(chunk_1_begin, chunk_2_begin))
    {
        size_t *chunk_1_end = (size_t *)((char *)chunk_1_begin + CHUNK_SIZE_WITHOUT_FLAGS(*chunk_1_begin));
        size_t *chunk_2_end = (size_t *)((char *)chunk_2_begin + CHUNK_SIZE_WITHOUT_FLAGS(*chunk_2_begin));
        PRINT_FAIL_TEST_AT(file_name, line_n);
        print_str(RED ": Memory at [");
        print_address_hex(chunk_1_begin);
        print_str(" - ");
        print_address_hex(chunk_1_end);
        print_str("] and [");
        print_address_hex(chunk_2_begin);
        print_str(" - ");
        print_address_hex(chunk_2_end);
        print_str("] is overlapping" NO_COLOR);
        print_endl();
        exit(1);
    }
}

void assert_alignment(void *ptr, char *file_name, int line_n)
{
    if ((((unsigned long) (ptr)) % BYTE_ALIGNMENT) != 0)
    {
        PRINT_FAIL_TEST_AT(file_name, line_n);
        print_str(RED ": pointer ");
        print_address_hex(ptr);
        print_str("is not aligned" NO_COLOR);
        print_endl();
        exit(1);
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

void visualise_memory(void)
{
    for (zone_header_t *tiny_zone = heap_g.tiny_zones_head; tiny_zone != NULL; tiny_zone = tiny_zone->next)
    {
        print_str("| ");
        print_address_hex(tiny_zone);
        print_str(GREEN " tiny zone header (");
        print_size(ZONE_HEADER_T_SIZE);
        print_str(" bytes): ");
        print_address_hex(tiny_zone->prev);
        print_str(" ");
        print_address_hex(tiny_zone->next);
        print_str(NO_COLOR " |\n");

        size_t *chunk_ptr = (size_t *)((uint8_t *)tiny_zone + ZONE_HEADER_T_SIZE);
        //size_t chunk_size = CHUNK_SIZE_WITHOUT_FLAGS(*chunk_ptr);
        for (size_t i = 0; i < ((heap_g.tiny_zone_size - ZONE_HEADER_T_SIZE) / heap_g.tiny_zone_chunk_max_size); i++)
        {
            size_t chunk_size = CHUNK_SIZE_WITHOUT_FLAGS(*chunk_ptr);
            uint8_t in_use_bit_flag = (uint8_t)(*chunk_ptr & IN_USE);
            uint8_t prev_free_bit_flag = (uint8_t)(*chunk_ptr & PREVIOUS_FREE);
            if ((*chunk_ptr & IN_USE) == IN_USE)
            {
                print_str("| ");
                print_address_hex(chunk_ptr);
                print_str(BLUE " chunk header (");
                print_size(SIZE_T_SIZE);
                print_str(" bytes): ");
                print_size(chunk_size);
                print_str(" ");
                print_size(prev_free_bit_flag);
                print_size(in_use_bit_flag);
                print_str(NO_COLOR " | ");
                print_str(MAGENTA "in use body (");
                print_size(chunk_size - SIZE_T_SIZE);
                print_str(" bytes)" NO_COLOR " |\n");
            }
            else
            {
                free_chunk_header_t *free_chunk_ptr = (free_chunk_header_t *)chunk_ptr;
                print_str("| ");
                print_address_hex(chunk_ptr);
                print_str(BLUE " chunk header (");
                print_size(sizeof(free_chunk_header_t));
                print_str(" bytes): ");
                print_size(chunk_size);
                print_str(" ");
                print_size(prev_free_bit_flag);
                print_size(in_use_bit_flag);
                print_str(" ");
                print_address_hex(free_chunk_ptr->prev);
                print_str(" ");
                print_address_hex(free_chunk_ptr->next);
                print_str(NO_COLOR " | ");
                print_str(YELLOW "free bytes (");
                print_size(chunk_size - sizeof(free_chunk_header_t));
                print_str(" bytes)" NO_COLOR " |\n");
            }
            chunk_ptr = (size_t *)((uint8_t *)(chunk_ptr) + chunk_size);
        }
    }
    for (zone_header_t *small_zone = heap_g.small_zones_head; small_zone != NULL; small_zone = small_zone->next)
    {
        print_str("| ");
        print_address_hex(small_zone);
        print_str(GREEN " small zone header (");
        print_size(ZONE_HEADER_T_SIZE);
        print_str(" bytes): ");
        print_address_hex(small_zone->prev);
        print_str(" ");
        print_address_hex(small_zone->next);
        print_str(NO_COLOR " |\n");

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
                print_str("| ");
                print_address_hex(chunk_ptr);
                print_str(BLUE " chunk header (");
                print_size(SIZE_T_SIZE);
                print_str(" bytes): ");
                print_size(chunk_size);
                print_str(" ");
                print_size(prev_free_bit_flag);
                print_size(in_use_bit_flag);
                print_str(NO_COLOR " | ");
                print_str(MAGENTA "in use body (");
                print_size(in_use_bytes);
                print_str(" bytes)" NO_COLOR " |\n");
            }
            else
            {
                free_chunk_header_t *free_chunk_ptr = (free_chunk_header_t *)chunk_ptr;
                size_t *footer_size = (size_t *)((uint8_t *)chunk_ptr + chunk_size - SIZE_T_SIZE);
                size_t free_bytes = chunk_size - sizeof(free_chunk_header_t) - SIZE_T_SIZE;
                print_str("| ");
                print_address_hex(chunk_ptr);
                print_str(BLUE " chunk header (");
                print_size(sizeof(free_chunk_header_t));
                print_str(" bytes): ");
                print_size(chunk_size);
                print_str(" ");
                print_size(prev_free_bit_flag);
                print_size(in_use_bit_flag);
                print_str(" ");
                print_address_hex(free_chunk_ptr->prev);
                print_str(" ");
                print_address_hex(free_chunk_ptr->next);
                print_str(NO_COLOR " | " YELLOW "free bytes (");
                print_size(free_bytes);
                print_str(" bytes)" NO_COLOR " | " BLUE "size (");
                print_size(SIZE_T_SIZE);
                print_str(" bytes): ");
                print_size(*footer_size);
                print_str(NO_COLOR " |\n");
            }
            chunk_ptr = (size_t *)((uint8_t *)(chunk_ptr) + chunk_size);
        }
    }  

    for (zone_header_t *large_zone = heap_g.large_zones_head; large_zone != NULL; large_zone = large_zone->next)
    {
        print_str("| ");
        print_address_hex(large_zone);
        print_str(GREEN " large zone header (");
        print_size(ZONE_HEADER_T_SIZE);
        print_str(" bytes): ");
        print_address_hex(large_zone->prev);
        print_str(" ");
        print_address_hex(large_zone->next);
        print_str(NO_COLOR " |\n");

        size_t *chunk_ptr = (size_t *)((uint8_t *)large_zone + ZONE_HEADER_T_SIZE);
        size_t chunk_size = CHUNK_SIZE_WITHOUT_FLAGS(*chunk_ptr);
        uint8_t in_use_bit_flag = (uint8_t)(*chunk_ptr & IN_USE);
        uint8_t prev_free_bit_flag = (uint8_t)(*chunk_ptr & PREVIOUS_FREE);
        if ((*chunk_ptr & IN_USE) == IN_USE)
        {
            size_t in_use_bytes = chunk_size - SIZE_T_SIZE;
            print_str("| ");
            print_address_hex(chunk_ptr);
            print_str(BLUE " chunk header (");
            print_size(SIZE_T_SIZE);
            print_str(" bytes): ");
            print_size(chunk_size);
            print_str(" ");
            print_size(prev_free_bit_flag);
            print_size(in_use_bit_flag);
            print_str(NO_COLOR " | " MAGENTA "in use body (");
            print_size(in_use_bytes);
            print_str(" bytes)" NO_COLOR " |\n");
        }
        else // should never go in here
        {
            free_chunk_header_t *free_chunk_ptr = (free_chunk_header_t *)chunk_ptr;
            size_t free_bytes = chunk_size - sizeof(free_chunk_header_t) - SIZE_T_SIZE;
            print_str("| ");
            print_address_hex(chunk_ptr);
            print_str(BLUE " chunk header (");
            print_size(sizeof(free_chunk_header_t));
            print_str(" bytes): ");
            print_size(chunk_size);
            print_str(" ");
            print_size(prev_free_bit_flag);
            print_size(in_use_bit_flag);
            print_str(" ");
            print_address_hex(free_chunk_ptr->prev);
            print_str(" ");
            print_address_hex(free_chunk_ptr->next);
            print_str(NO_COLOR " | " YELLOW "free bytes (");
            print_size(free_bytes);
            print_str(" bytes)" NO_COLOR " |\n");
        }
    }
}
