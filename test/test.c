#include <stdio.h>
#include <unistd.h>
#include <sys/resource.h>
#include "malloc.h"
#include "heap.h"

int main()
{
	struct rlimit rlimit;
	getrlimit(RLIMIT_DATA, &rlimit);
	printf("Hello. Pagesize: %d, soft data limit: %llu, hard: %llu\n", getpagesize(), rlimit.rlim_cur, rlimit.rlim_max);
	printf("free chunk header size: %zu bytes\n", sizeof(free_chunk_header_t));
	printf("zone header size: %zu bytes\n", sizeof(zone_header_t));

	char *ptr_1 = malloc(131073);
	char *ptr_2 = malloc(131090);
	void *ptr_3 = malloc(2);
	void *ptr_4 = malloc(12);
	show_alloc_mem();

	free(ptr_2);
	free(ptr_3);
	ptr_3 = malloc(2);
	void *ptr_5 = malloc(12);

	(void)ptr_1;
	(void)ptr_2;
	(void)ptr_3;
	(void)ptr_4;
	(void)ptr_5;

	show_alloc_mem();
}
