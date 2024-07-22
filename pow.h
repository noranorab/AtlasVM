#ifndef POW_H
#define POW_H
#include <stddef.h>
#include <stdint.h>

int compute_pow(uint8_t* block, size_t block_size, uint32_t difficulty);

#endif // POW_H