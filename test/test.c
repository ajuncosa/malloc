#include <stdio.h>
#include "malloc.h"

int main()
{
	printf("Hello\n");
	void *ptr = malloc(3);
	(void)ptr;
}