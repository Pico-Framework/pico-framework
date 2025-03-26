#include "pico/stdlib.h"
#include <cstring>
#include "FreeRTOS.h"
#include <stdio.h>

void* operator new(size_t size) {
    void* pData = pvPortMalloc(size);
    if (!pData) {
        printf("ERROR: Memory allocation failed for size %zu!\n", size);
        while(1);  // Trap in infinite loop to catch memory failures
    }
    memset(pData, 0, size);
    return pData;
}

void * operator new[]( size_t size ){
    return pvPortMalloc(size);
}

void operator delete(void *ptr) {
    if (!ptr) {
        printf("ERROR: Attempted to delete a NULL pointer!\n");
        return;
    }
//    printf("Freeing memory at address: %p\n", ptr);
    vPortFree(ptr);
}


