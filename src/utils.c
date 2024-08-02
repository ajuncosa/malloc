#include "utils.h"

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

void print_endl(void)
{
	write(1, "\n", 1);
}
