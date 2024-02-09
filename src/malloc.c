#include "malloc.h"
#include <stdio.h>

void *malloc(size_t size)
{
	void *ptr = NULL;
	printf("mallocing %zu bytes\n", size);
	return ptr;
}