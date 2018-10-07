#include <stdint.h>

#define RAND_MAX 0xffff
static uint32_t rand_seed = 1;

uint32_t random() {
    rand_seed = rand_seed * 1103515245 + 12345;
    return rand_seed & RAND_MAX;
}
