#include "test_utils.h"
#include "malloc.h"

int main()
{
	char *test_name = "test_large_realloc_1";
    size_t malloc_size = SMALL_ZONE_MAX_CHUNK_SIZE - SIZE_T_SIZE + 1;

	char *ptr = malloc(malloc_size);
	ASSERT_POINTER_NE(ptr, NULL);
	for(size_t i = 0; i < malloc_size; i++)
		ptr[i] = 'a' + i % 26;

	ASSERT_SIZE_EQ(zone_list_len(heap_g.large_zones_head), 1);
    
    char *new_ptr = realloc(ptr, 1000000);
	ASSERT_POINTER_NE(new_ptr, NULL);
	ASSERT_POINTER_NE(new_ptr, ptr);
	for(size_t i = 0; i < malloc_size; i++)
		ASSERT_CHAR_EQ(new_ptr[i], 'a' + i % 26);

	ASSERT_SIZE_EQ(zone_list_len(heap_g.large_zones_head), 1);

	free(new_ptr);

	ASSERT_SIZE_EQ(zone_list_len(heap_g.large_zones_head), 0);

	pass_test(test_name);
}
