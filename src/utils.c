#include <stdint.h>
#include "utils.h"

static void print_hex_number(size_t n)
{
    if (n >= 16)
        print_hex_number(n / 16);
    
    char c = n % 16;
    c += c > 9 ? ('a' - 10) : '0';
    write(1, &c, 1);
}

void print_address_hex(void *ptr)
{
    uintptr_t p = (uintptr_t)ptr;

    write(1, "0x", 2);
    print_hex_number(p);
}

void print_str(char *s)
{
	if (s == NULL)
		return ;

    for (size_t i = 0; s[i] != '\0'; i++)
		write(1, &s[i], 1);
}

void print_size(size_t n)
{
	if (n / 10 > 0)
		print_size(n / 10);

	char c = (n % 10) + '0';
	write(1, &c, 1);
}

void print_endl(void)
{
	write(1, "\n", 1);
}
void print_chunk_info(void *begin, void *end, size_t bytes)
{
    print_str("  ");
    print_address_hex(begin);
    print_str(" - ");
    print_address_hex(end);
    print_str(": ");
    print_size(bytes);
    print_str(" bytes");
    print_endl();
}

void *ft_memcpy(void *dst, const void *src, size_t n)
{
	char		*tmpdst;
	const char	*tmpsrc;

	if (dst == src)
		return (dst);

	tmpdst = (char *)dst;
	tmpsrc = (const char *)src;
	for (size_t i = 0; i < n; i++)
		tmpdst[i] = tmpsrc[i];

	return (dst);
}

void ft_putchar(char c)
{
	write(1, &c, 1);
}
