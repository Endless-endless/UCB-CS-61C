#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "lfsr.h"

void lfsr_calculate(uint16_t *reg) {
    /* YOUR CODE HERE */
    unsigned b0 = *reg & 1;
    unsigned b2 = (*reg >> 2) & 1;
    unsigned b3 = (*reg >> 3) & 1;
    unsigned b5 = (*reg >> 5) & 1;
    unsigned fb = b0 ^ b2 ^ b3 ^ b5;

    unsigned short res = *reg >> 1;

    res = res | (fb << 15);

    *reg = res;
}

