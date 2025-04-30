/**
 * @file utility.cpp
 * @author Ian Archbell
 * @brief Implementation of diagnostics and utility functions for memory, stack, and lwIP state.
 *
 * Provides runtime tools for embedded system visibility including FreeRTOS and lwIP stats,
 * memory usage, and debug trace control.
 *
 * @version 0.1
 * @date 2025-03-26
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

#include "framework_config.h" // Must be included before DebugTrace.h to ensure framework_config.h is processed first
#include "DebugTrace.h"
TRACE_INIT(utility) // Initialize tracing for this module

#include "utility/utility.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <algorithm>
#include <sstream>
#include "pico/stdlib.h"

#ifndef UNIT_TEST
#include <lwip/memp.h>
#endif
#include <lwip/stats.h>
#include <lwip/pbuf.h>
#include <lwip/sockets.h>
#include <lwip/ip_addr.h>

/// @copydoc printTaskStackSizes
void printTaskStackSizes()
{
    TRACE("Task Stack Sizes:\n");

    TaskStatus_t *pxTaskStatusArray;
    UBaseType_t uxTaskCount = uxTaskGetNumberOfTasks();
    pxTaskStatusArray = (TaskStatus_t *)pvPortMalloc(uxTaskCount * sizeof(TaskStatus_t));

    if (pxTaskStatusArray != NULL)
    {
        uxTaskGetSystemState(pxTaskStatusArray, uxTaskCount, NULL);
        for (UBaseType_t i = 0; i < uxTaskCount; i++)
        {
            TRACE("Task %s: Stack high watermark: %u bytes\n",
                  pxTaskStatusArray[i].pcTaskName,
                  pxTaskStatusArray[i].usStackHighWaterMark);
        }
        vPortFree(pxTaskStatusArray);
    }
    else
    {
        TRACE("Failed to allocate memory for task status array.\n");
    }
}

/// @copydoc logSystemStats
void logSystemStats()
{
    printf("\n===== SYSTEM STATS =====\n");
    printf("Free heap size: %zu bytes\n", xPortGetFreeHeapSize());
    printf("Minimum ever free heap: %zu bytes\n", xPortGetMinimumEverFreeHeapSize());
    printf("Stack watermark: AcceptConnect: %zu, HandleClient: %zu, tcpip_thread: %zu\n",
           uxTaskGetStackHighWaterMark(NULL),
           uxTaskGetStackHighWaterMark(NULL),
           uxTaskGetStackHighWaterMark(NULL));
    printf("========================\n");
}

/// @copydoc printHeapInfo
void printHeapInfo()
{
    TRACE("Heap Information:\n");
    TRACE("Free heap size: %u bytes\n", (unsigned int)xPortGetFreeHeapSize());
    TRACE("Minimum ever free heap size: %u bytes\n", (unsigned int)xPortGetMinimumEverFreeHeapSize());
}

/// @copydoc printSystemMemoryInfo
void printSystemMemoryInfo()
{
    printTaskStackSizes();
    printHeapInfo();
}

/// @copydoc toLower
std::string toLower(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c)
                   { return std::tolower(c); });
    return str;
}

/// @copydoc runTimeStats
void runTimeStats()
{
    TaskStatus_t *pxTaskStatusArray;
    UBaseType_t uxArraySize = uxTaskGetNumberOfTasks();
    unsigned long ulTotalRunTime;

    TRACE("Number of tasks %d\n", uxArraySize);
    pxTaskStatusArray = (TaskStatus_t *)pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));

    if (pxTaskStatusArray != NULL)
    {
        uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &ulTotalRunTime);

        for (UBaseType_t x = 0; x < uxArraySize; x++)
        {
            printf("Task: %d \t cPri:%d \t bPri:%d \t hw:%zu \t%s \t core: %s\n",
                   pxTaskStatusArray[x].xTaskNumber,
                   pxTaskStatusArray[x].uxCurrentPriority,
                   pxTaskStatusArray[x].uxBasePriority,
                   pxTaskStatusArray[x].usStackHighWaterMark,
                   pxTaskStatusArray[x].pcTaskName);
            printf("%d\n", get_core_num());
        }
        vPortFree(pxTaskStatusArray);
    }
    else
    {
        panic("Failed to allocate space for stats\n");
    }

    HeapStats_t heapStats;
    vPortGetHeapStats(&heapStats);
    printf("HEAP avl: %zu, blocks %zu, alloc: %zu, free: %zu\n",
           heapStats.xAvailableHeapSpaceInBytes,
           heapStats.xNumberOfFreeBlocks,
           heapStats.xNumberOfSuccessfulAllocations,
           heapStats.xNumberOfSuccessfulFrees);
}

/// @copydoc printActivePCBs
void printActivePCBs()
{
#if LWIP_STATS && MEMP_STATS
    TRACE("Active TCP PCBs: %d\n", lwip_stats.memp[MEMP_TCP_PCB]->used);
#else
    TRACE("LWIP stats are not enabled, cannot track active PCBs.\n");
#endif
}

/// @copydoc printTCPState
void printTCPState()
{
    TRACE("\n===== TCP/IP STATE =====\n");
#if LWIP_STATS && TRACE_INFO
    extern void stats_display(void);
    stats_display();
#else
    TRACE("lwIP stats unavailable. Ensure LWIP_STATS is enabled in lwipopts.h\n");
#endif
    TRACE("========================\n");
}

/// @copydoc printMemsize
void printMemsize()
{
    printf("MEM SIZE: %d\n", lwip_stats.mem.avail);
    printf("MEM USED: %d\n", lwip_stats.mem.used);
}

/// @copydoc is_in_interrupt
int is_in_interrupt(void)
{
    uint32_t ipsr_value;
#ifndef UNIT_TEST
    __asm volatile("MRS %0, ipsr" : "=r"(ipsr_value));
#else
    ipsr_value = 0;
#endif
    return (ipsr_value != 0);
}

/// @copydoc debug_print(const char *msg)
void debug_print(const char *msg)
{
    if (msg)
    {
        for (const char *p = msg; *p; ++p)
        {
            uart_putc(uart0, *p);
        }
    }
}
/// @copydoc debug_print(const std::string &msg)
void debug_print(const std::string &msg)
{
    for (const char *p = msg.c_str(); *p; ++p)
    {
        uart_putc(uart0, *p);
    }
}
/// @copydoc debug_print(const char *msg, size_t len)       
void debug_print(const char *msg, size_t len)
{
    if (msg && len > 0)
    {
        for (size_t i = 0; i < len; ++i)
        {
            uart_putc(uart0, msg[i]);
        }
    }
}

/// @copydoc debug_warning(const std::string &msg)
void warning(const std::string &msg)
{
    printf("\033[31m");
    printf("WARNING: %s\n", msg.c_str());
    printf("\033[0m");
}

/// @copydoc debug_warning(const std::string &msg)
void warning(const std::string &msg, int32_t code)
{
    std::string fullMsg = msg + " (code: " + std::to_string(code) + ")\n";
    printf("\033[31m");
    printf("WARNING: %s\n", fullMsg.c_str());
    printf("\033[0m");
}

#if PICO_RP2350
#include "RP2350.h"
#else
#include "RP2040.h"
#endif

void rebootSystem() {
    NVIC_SystemReset();
}


