#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/resource.h>
#include "malloc.h"
#include "heap.h"

#define M (1024 * 1024)

void print(char *s)
{
	write(1, s, strlen(s));
}

int main()
{
	struct rlimit rlimit;
	getrlimit(RLIMIT_DATA, &rlimit);
	printf("Hello. Pagesize: %d, soft data limit: %llu, hard: %llu\n", getpagesize(), rlimit.rlim_cur, rlimit.rlim_max);
	//printf("free chunk header size: %zu bytes\n", sizeof(free_chunk_header_t));
	//printf("zone header size: %zu bytes\n", sizeof(zone_header_t));
/*

	char *ptr_1 = malloc(131073);
	char *ptr_2 = malloc(131090);
	void *ptr_3 = malloc(2);
	void *ptr_4 = malloc(12);
	void *ptr_6 = malloc(33);
	show_alloc_mem();

	free(ptr_2);
	free(ptr_3);
	ptr_3 = malloc(2);
	void *ptr_5 = malloc(12);
	void *ptr_7 = malloc(131000);

	free(ptr_6);
	ptr_6 = malloc(100);

	(void)ptr_1;
	(void)ptr_2;
	(void)ptr_3;
	(void)ptr_4;
	(void)ptr_5;
	(void)ptr_6;
	(void)ptr_7;

	show_alloc_mem();
	*/
/*
	char *ptr_1 = malloc(33);
	char *ptr_2 = malloc(100);
	void *ptr_3 = malloc(200);
	void *ptr_4 = malloc(1000);
	void *ptr_5 = malloc(34);

	show_alloc_mem();

	free(ptr_2);
	free(ptr_3);
	show_alloc_mem();

	//ptr_3 = malloc(2);
	void *ptr_6 = malloc(200);
	void *ptr_7 = malloc(131000);
	void *ptr_8 = malloc(288 - SIZE_T_SIZE);
	void *ptr_9 = malloc(131072 - SIZE_T_SIZE);
	(void)ptr_1;
	(void)ptr_2;
	(void)ptr_3;
	(void)ptr_4;
	(void)ptr_5;
	(void)ptr_6;
	(void)ptr_7;
	(void)ptr_8;
	(void)ptr_9;
*/
/*
	char *ptr = NULL;
	for (size_t i = 0; i < 99 ; i++)
	{
		ptr = malloc(SMALL_ZONE_CHUNK_MAX_SIZE - SIZE_T_SIZE);
		if (ptr == NULL)
			exit(1);
		
		//ptr_1[SMALL_ZONE_CHUNK_MAX_SIZE - SIZE_T_SIZE -1] = 'a';
	}
*/
	/*
	char *ptr2 = malloc(SMALL_ZONE_CHUNK_MAX_SIZE - SIZE_T_SIZE);
	if (ptr2 == NULL)
		exit(1);

	free(ptr);
	free(ptr2);

	ptr = malloc(55);
	if (ptr == NULL)
		exit(1);

	(void)ptr2;
	*/
	/*
	for (size_t i = 0; i < 99 ; i++)
	{
		free(ptr);
		ptr -= SMALL_ZONE_CHUNK_MAX_SIZE;
	}
	// FIXME: esto deberia estar liberando la zona antes de alocar, pero
	// solo recorre un par de elementos en la unsorted (probablemente estoy
	// seteando el next a null al mover las cosas del unosrted)
	ptr = malloc(100);
	(void)ptr;
	show_alloc_mem();
	*/
/*
	char *ptr_1 = malloc(100);
	char *ptr_2 = malloc(200);
	void *ptr_3 = malloc(100);
	void *ptr_4 = malloc(200);
	void *ptr_5 = malloc(200);
	void *ptr_6 = malloc(100);
	char *ptr_7 = malloc(200);
	void *ptr_8 = malloc(100);
	void *ptr_9 = malloc(200);
	void *ptr_10 = malloc(100);

	free(ptr_2);
	show_alloc_mem();
	ptr_2 = malloc(400);
	show_alloc_mem();
	free(ptr_3);
	show_alloc_mem();
	free(ptr_4);
	free(ptr_5);
	ptr_3 = malloc(400);
	free(ptr_6);
	ptr_6 = malloc(100);
*/
/*
	char *ptr_1 = malloc(100);
	char *ptr_2 = malloc(100);
	void *ptr_3 = malloc(100);
	void *ptr_4 = malloc(100);
	void *ptr_5 = malloc(100);
	void *ptr_6 = malloc(100);
	char *ptr_7 = malloc(100);
	void *ptr_8 = malloc(100);
	void *ptr_9 = malloc(100);
	free(ptr_1);
	free(ptr_2);
	free(ptr_3);
	free(ptr_4);
	free(ptr_5);
	free(ptr_6);
	free(ptr_7);
	free(ptr_8);
	free(ptr_9);

	(void)ptr_1;
	(void)ptr_2;
	(void)ptr_3;
	(void)ptr_4;
	(void)ptr_5;
	(void)ptr_6;
	(void)ptr_7;
	(void)ptr_8;
	(void)ptr_9;
	//(void)ptr_10;
	ptr_1 = malloc(200);

	show_alloc_mem();
*/
/*
	int i;
	char *addr;

	i = 0;
	while (i < 1024)
	{
	addr = (char*)malloc(1024);
	addr[0] = 42;
	i++;
	free(addr);

	}
	addr = (char*)malloc(1024);
	show_alloc_mem();
	addr = realloc(addr, SMALL_ZONE_CHUNK_MAX_SIZE+1);
	show_alloc_mem();

*/
	char *addr1;
	char *addr2;
	char *addr3;

	addr1 = (char*)malloc(16*M);
	strcpy(addr1, "Bonjours\n");
	print(addr1);
	addr2 = (char*)malloc(16*M);
	addr3 = (char*)realloc(addr1, 128*M);
	addr3[127*M] = 42;
	print(addr3);

	(void)addr2;

/*
	char *addr;

	addr = malloc(16);
	free(NULL);
	free((void *)addr + 5);
	if (realloc((void *)addr + 5, 10) == NULL)
	print("Bonjours\n");

	char *addr1;
	addr1 = malloc(1024);
	addr1 = malloc(1024 * 32);
	addr1 = malloc(1024 * 1024);
	addr1 = malloc(1024 * 1024 * 16);
	addr1 = malloc(1024 * 1024 * 128);
	show_alloc_mem();
	(void)addr1;
*/
}
