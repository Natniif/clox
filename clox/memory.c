#include <stdlib.h>

#include "memory.h"

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    if (newSize == 0) {
        free(pointer);
        return NULL;
    }

    // realloc changes size of storage and points pointer to start of this storage
    void* result = realloc(pointer, newSize);

    // if not enough memory to allocate
    if (result == NULL) exit(1);
    return result;
}