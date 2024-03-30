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
	printf("zone header size: %zu bytes\n", sizeof(zone_t));

	char *ptr_1 = malloc(131073);
	char *ptr_2 = malloc(131090);
	//char *ptr_1 = malloc(4097);
	//void *ptr_2 = malloc(5);
	//void *ptr_3 = malloc(100);
	show_alloc_mem();

	free(ptr_2);

	(void)ptr_1;
	(void)ptr_2;
	//(void)ptr_3;

	show_alloc_mem();
}
