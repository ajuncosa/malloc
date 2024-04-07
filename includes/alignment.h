#ifndef ALIGNMENT_H
#define ALIGNMENT_H

#define BYTE_ALIGNMENT          16
#define ALIGN(size)	            (((size) + (BYTE_ALIGNMENT - 1)) & ~(BYTE_ALIGNMENT - 1))
#define SIZE_T_SIZE	            (ALIGN(sizeof(size_t))) // in-use chunk header metadata size

#endif
