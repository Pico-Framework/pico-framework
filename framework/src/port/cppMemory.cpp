/**
 * @file cppMemory.cpp
 * @author Ian Archbell
 * @brief Global `new`/`delete` operators using FreeRTOS memory routines.
 *
 * Provides replacements for C++ global `new`, `new[]`, `delete`, and `delete[]`
 * operators to use FreeRTOS `pvPortMalloc` and `vPortFree` in embedded builds.
 * Also includes placement new for STL compatibility.
 *
 * @version 0.1
 * @date 2025-03-31
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

 #include "pico/stdlib.h"
 #include <cstring>
 #include "FreeRTOS.h"
 #include <stdio.h>
 
 #ifndef UNIT_TEST
 
 void* operator new(size_t size) {
     void* pData = pvPortMalloc(size);
     if (!pData) {
         printf("ERROR: Memory allocation failed for size %zu!\n", size);
         while(1);  // Trap in infinite loop to catch memory failures
     }
     memset(pData, 0, size);
     return pData;
 }
 
 void* operator new[](size_t size) {
     return pvPortMalloc(size);
 }
 
 void operator delete(void* ptr) {
     if (ptr) {
         vPortFree(ptr);
     }
 }
 
 void operator delete[](void* ptr) {
     if (ptr) {
         vPortFree(ptr);
     }
 }
 
 // Placement new — required for STL
 void* operator new(size_t, void* ptr) noexcept {
     return ptr;
 }
 
 void* operator new[](size_t, void* ptr) noexcept {
     return ptr;
 }
 
 #endif
 