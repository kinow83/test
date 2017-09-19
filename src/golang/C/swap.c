#include <stdio.h>
#include "_cgo_export.h"

void CExample() {
    struct swap_return s;
    s = swap(3, 5);
    printf("%d %d\n", s.r0, s.r1);
}