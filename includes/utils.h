#ifndef UTILS_H
#define UTILS_H

#include <unistd.h>

void print_address_hex(void *ptr);
void print_str(char *s);
void print_size(size_t n);
void print_endl(void);
void print_chunk_info(void *begin, void *end, size_t bytes);
void *ft_memcpy(void *dst, const void *src, size_t n);
void ft_putchar(char c);

#endif
