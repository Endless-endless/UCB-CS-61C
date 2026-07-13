#include <stdio.h>
int main() {
    int a[5] = {1, 2, 3, 4, 5};
    unsigned total = 0;
    int len = sizeof(a) / sizeof(int);
    for (int j = 0; j < len; j++) {
        total += a[j];
    }
    printf("sum of array is %d\n", total);
}