#ifndef UTILS_H
#define UTILS_H

#include <unistd.h>

void print_address_hex(void *ptr);
void *ft_memcpy(void *dst, const void *src, size_t n);
void print_str(char *s);
void print_endl(void);
void print_size_t(size_t n);

/*
size_t ft_strlen(const char *s);
void	ft_putchar_fd(char c, int fd);
void	ft_putnbr_fd(int n, int fd);
*/

#endif
