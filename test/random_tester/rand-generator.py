import sys
from datetime import datetime
import random

def generate_allocaiton_file(file_name: str, n_allocations: int, n_reallocations: int, lower_bound: int, upper_bound: int, offset: int):

    allocations = []

    for i in range(0, n_allocations):
        size = random.randint(lower_bound, upper_bound)
        alloc = {"id": i, "size": size}
        allocations.append(alloc)

    random.shuffle(allocations)

    available_allocations = []
    available_reallocations = []

    generating = True
    i = 0
    n = 0
    created_reallocations = 0
    created_frees = 0

    file = open(file_name, "w")
    file.write(f"{str(n_allocations).rjust(29)}\n")
    while generating:

        if i < len(allocations):
            allocation = allocations[i]
            file.write(f"M {str(allocation['id']).rjust(20)} {str(allocation['size']).rjust(20)}\n")
            available_reallocations.append(allocation["id"])
            i += 1
        
        if n < offset:
            n += 1
            continue

        remaining_allocations = len(allocations) - i
        create_reallocatio_rand = created_reallocations < n_reallocations and remaining_allocations > n_reallocations
        create_reallocation = random.randint(0, 2) if create_reallocatio_rand else (True and created_reallocations < n_reallocations)
        if create_reallocation:
            if len(available_reallocations) > 0:
                tmp_rand = available_reallocations.copy()
                random.shuffle(tmp_rand)
                alloc_id = tmp_rand.pop(0)
                available_reallocations.remove(alloc_id)
                size = random.randint(lower_bound, upper_bound)
                file.write(f"R {str(alloc_id).rjust(20)} {str(size).rjust(20)}\n")
                created_reallocations += 1
                available_allocations.append(alloc_id)
        else:
            tmp_rand = available_reallocations.copy()
            random.shuffle(tmp_rand)
            alloc_id = tmp_rand.pop(0)
            available_reallocations.remove(alloc_id)
            available_allocations.append(alloc_id)

        if created_frees < n_allocations and len(available_allocations) > 0:
            tmp_free = available_allocations.copy()
            random.shuffle(tmp_free)
            allocation_id = tmp_free.pop()
            available_allocations.remove(allocation_id)
            file.write(f"F {str(allocation_id).rjust(20)} {str(' ').rjust(20)}\n")
            created_frees += 1
        
        #print(f"allocations: {i}/{n_allocations} reallocations: {created_reallocations}/{n_reallocations} frees: {created_frees}/{n_allocations}")

        if i == n_allocations and created_reallocations == n_reallocations and created_frees == n_allocations:
            generating = False

    file.close()

if __name__ == "__main__":
    if len(sys.argv) == 6:
        print("Invalid number of arguments")
        exit(1)

    generate_allocaiton_file(str(sys.argv[1]), int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4]), int(sys.argv[5]), int(sys.argv[6]))
