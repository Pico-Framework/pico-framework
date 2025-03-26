/**
 * @file utility.cpp
 * @author Ian Archbell
 * @brief 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <algorithm>
#include "lwip/memp.h"
#include "lwip/stats.h"
#include "lwip/pbuf.h"

// Function to print stack sizes of all tasks
void printTaskStackSizes() {
    TRACE("Task Stack Sizes:\n");

    // Get the list of all tasks in the system
    TaskStatus_t *pxTaskStatusArray;
    UBaseType_t uxTaskCount;
    uxTaskCount = uxTaskGetNumberOfTasks();
    
    // Allocate space to hold task status information
    pxTaskStatusArray = (TaskStatus_t*)pvPortMalloc(uxTaskCount * sizeof(TaskStatus_t));

    if (pxTaskStatusArray != NULL) {
        // Get task status information
        uxTaskGetSystemState(pxTaskStatusArray, uxTaskCount, NULL);

        // Iterate over each task and print stack usage
        for (UBaseType_t i = 0; i < uxTaskCount; i++) {
            TRACE("Task %s: Stack high watermark: %u bytes\n",
                pxTaskStatusArray[i].pcTaskName,
                pxTaskStatusArray[i].usStackHighWaterMark);
        }

        // Free the allocated memory
        vPortFree(pxTaskStatusArray);
    } else {
        TRACE("Failed to allocate memory for task status array.\n");
    }
}

void logSystemStats() {
    printf("\n===== SYSTEM STATS =====\n");
    printf("Free heap size: %d bytes\n", xPortGetFreeHeapSize());
    printf("Minimum ever free heap: %d bytes\n", xPortGetMinimumEverFreeHeapSize());

    printf("Stack watermark: AcceptConnect: %d, HandleClient: %d, tcpip_thread: %d\n",
        uxTaskGetStackHighWaterMark(NULL),  // Accept thread
        uxTaskGetStackHighWaterMark(NULL),  // HandleClient thread
        uxTaskGetStackHighWaterMark(NULL)); // TCP/IP thread

    printf("========================\n");
}

// Function to print heap information
void printHeapInfo() {
    size_t heapSize = xPortGetFreeHeapSize();
    size_t minHeapSize = xPortGetMinimumEverFreeHeapSize();

    TRACE("Heap Information:\n");
    TRACE("Free heap size: %u bytes\n", (unsigned int)heapSize);
    TRACE("Minimum ever free heap size: %u bytes\n", (unsigned int)minHeapSize);
}

// Function to print stack and heap information
void printSystemMemoryInfo() {
    printTaskStackSizes();
    printHeapInfo();
}

// Convert string to lowercase for case-insensitive lookup
std::string toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return str;
}

void runTimeStats(   ){
	TaskStatus_t *pxTaskStatusArray;
	volatile UBaseType_t uxArraySize, x;
	unsigned long ulTotalRunTime;


   // Get number of takss
   uxArraySize = uxTaskGetNumberOfTasks();
   TRACE("Number of tasks %d\n", uxArraySize);

   //Allocate a TaskStatus_t structure for each task.
   pxTaskStatusArray = (TaskStatus_t *)pvPortMalloc( uxArraySize * sizeof( TaskStatus_t ) );

   if( pxTaskStatusArray != NULL ){
		// Generate raw status information about each task.

		uxArraySize = uxTaskGetSystemState( pxTaskStatusArray,
									uxArraySize,
									&ulTotalRunTime );

		// Print stats
		for( x = 0; x < uxArraySize; x++ )
		{
			printf("Task: %d \t cPri:%d \t bPri:%d \t hw:%d \t%s\n",
					pxTaskStatusArray[ x ].xTaskNumber ,
					pxTaskStatusArray[ x ].uxCurrentPriority ,
					pxTaskStatusArray[ x ].uxBasePriority ,
					pxTaskStatusArray[ x ].usStackHighWaterMark ,
					pxTaskStatusArray[ x ].pcTaskName
					);
		}
		// Free array
		vPortFree( pxTaskStatusArray );
   } else {
	   panic("Failed to allocate space for stats\n");
   }

   //Get heap allocation information
   HeapStats_t heapStats;
   vPortGetHeapStats(&heapStats);
   printf("HEAP avl: %d, blocks %d, alloc: %d, free: %d\n",
		   heapStats.xAvailableHeapSpaceInBytes,
		   heapStats.xNumberOfFreeBlocks,
		   heapStats.xNumberOfSuccessfulAllocations,
		   heapStats.xNumberOfSuccessfulFrees
		   );
}

void printActivePCBs() {
#if LWIP_STATS && MEMP_STATS
    TRACE("Active TCP PCBs: %d\n", lwip_stats.memp[MEMP_TCP_PCB]->used);
#else
    TRACE("LWIP stats are not enabled, cannot track active PCBs.\n");
#endif
}

#if LWIP_STATS && TRACE_INFO
    extern void stats_display(void);
#endif

void printTCPState() {
    TRACE("\n===== TCP/IP STATE =====\n");

    #if LWIP_STATS && TRACE_INFO
        stats_display();  // Print all available lwIP stats
    #else
        TRACE("lwIP stats unavailable. Ensure LWIP_STATS is enabled in lwipopts.h\n");
    #endif

    TRACE("========================\n");
}

void vgetTaskDetails( TaskStatus_t* xTaskDetails )
{
    TaskHandle_t xHandle;

    /* Obtain the handle of a task from its name. */
    xHandle = xTaskGetHandle( "Task_Name" );

    /* Check the handle is not NULL. */
    configASSERT( xHandle );

    /* Use the handle to obtain further information about the task. */
    vTaskGetInfo( /* The handle of the task being queried. */
                  xHandle,
                  /* The TaskStatus_t structure to complete with information
                     on xTask. */
                  xTaskDetails,
                  /* Include the stack high water mark value in the
                     TaskStatus_t structure. */
                  pdTRUE,
                  /* Include the task state in the TaskStatus_t structure. */
                  eInvalid );
}

#include "lwip/pbuf.h"
#include "lwip/memp.h"
#include "lwip/tcp.h"
#include <stdio.h>

// Define the MEM_SIZE calculation
#define PBUF_POOL_BUFSIZE_CALC (TCP_MSS + 40)

void printMemsize() {
    printf("MEM SIZE: %d\n", lwip_stats.mem.avail);
    printf("MEM USED: %d\n", lwip_stats.mem.used); 
}

// Function to check if in interrupt context
int is_in_interrupt(void) {
    uint32_t ipsr_value;
    // Read the IPSR value using inline assembly
    __asm volatile ("MRS %0, ipsr" : "=r" (ipsr_value));
      
    // Return 1 if in interrupt, 0 if not
    return (ipsr_value != 0);
  }
