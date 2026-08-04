#include "transpose.h"

/* The naive transpose function as a reference. */
void transpose_naive(int n, int blocksize, int *dst, int *src) {
    for (int x = 0; x < n; x++) {
        for (int y = 0; y < n; y++) {
            dst[y + x * n] = src[x + y * n];
        }
    }
}

/* Implement cache blocking below. You should NOT assume that n is a
 * multiple of the block size. */
void transpose_blocking(int n, int blocksize, int *dst, int *src) {
    // YOUR CODE HERE
    for (int bi = 0; bi < n; bi += blocksize)
    {
        for (int bj = 0; bj < n; bj += blocksize)
        {
            for (int x = 0; x < blocksize && (x + bi) < n; x++)
            {
                for (int y = 0; y < blocksize && (y + bj) < n; y++)
                {
                    int r = bi + x;
                    int c = bj + y;
                    dst[ r + c * n] = src[ c + r * n];
                }
            }
        }
    }
}
