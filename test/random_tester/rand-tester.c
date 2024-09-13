#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include "test_utils.h"
#include "utils.h"

int main(int argc, char **argv) {

    if (argc != 2) {
        print_str("invalid arguments :(\n");
        return 1;
    }
    
    int file = open(argv[1], 0);

    if (file == -1) {
        print_str("invalid file\n");
        exit(1);
    }

    char parsed_n_allocations[31];
    memset(parsed_n_allocations, 0, 30);
    read(file, &parsed_n_allocations, 30);
    size_t n_allocations = atoi(parsed_n_allocations);
    char *allocations[n_allocations];

    int rd = 1;
    //int i=0;
    while (rd > 0) {
        char c;
        char s_id[22];
        char s_size[22];
        char nl;

        memset(s_id, 0, 22);
        memset(s_size, 0, 22);
        
        s_id[21] = 0;
        s_size[21] = 0;

        rd = read(file, &c, 1);
        rd = read(file, &s_id, 21);
        rd = read(file, &s_size, 21);
        rd = read(file, &nl, 1);


        if (rd == 0)
            break;

        int id = atoi(s_id);
        int size = atoi(s_size);

        if (c == 'M') {
            ft_putchar(c);
            ft_putchar(' ');
            print_size(id);
            ft_putchar(' ');
            print_size(size);
            print_endl();
            allocations[id] = malloc(size);

            if (size == 0) {
                continue;
            }

            if (size >= 8) {
                allocations[id][0] = c;
                allocations[id][1] = c;
                allocations[id][2] = c;
                allocations[id][3] = c;
                int *int_pos = (int *)(&allocations[id][4]);
                *int_pos = size;
            }

            if (size >= 16) {
                allocations[id][size - 1] = c;
                allocations[id][size - 2] = c;
                allocations[id][size - 3] = c;
                allocations[id][size - 4] = c;
                int *int_pos = (int *)(&allocations[id][size - 8]);
                *int_pos = size;
            }
            
            if (size < 8) {
                allocations[id][0] = 'M';
            }
        }
        else if (c == 'R') {
            ft_putchar(c);
            ft_putchar(' ');
            print_size(id);
            ft_putchar(' ');
            print_size(size);
            print_endl();

            allocations[id] = realloc(allocations[id], size);
            if (allocations[id] == NULL)
                continue;
            if (size == 0) {
                continue;
            }
            
            if (size >= 8) {
                allocations[id][0] = c;
                allocations[id][1] = c;
                allocations[id][2] = c;
                allocations[id][3] = c;
                int *int_pos = (int *)(&allocations[id][4]);
                *int_pos = size;
            }

            if (size >= 16) {
                allocations[id][size - 1] = c;
                allocations[id][size - 2] = c;
                allocations[id][size - 3] = c;
                allocations[id][size - 4] = c;
                int *int_pos = (int *)(&allocations[id][size - 8]);
                *int_pos = size;
            }
            
            if (size < 8) {
                allocations[id][0] = 'M';
            }
        }
        else if (c == 'F') {
            ft_putchar(c);
            ft_putchar(' ');
            print_size(id);
            print_endl();

            free(allocations[id]);
        }

    }

    close(file);

    return 0;
}