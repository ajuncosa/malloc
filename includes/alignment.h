#ifndef ALIGNMENT_H
#define ALIGNMENT_H

#define BYTE_ALIGNMENT  (2 * sizeof(size_t))
#define ALIGN(size)     (((size) + (BYTE_ALIGNMENT - 1)) & ~(BYTE_ALIGNMENT - 1))

#endif
