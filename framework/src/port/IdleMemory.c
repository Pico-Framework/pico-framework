/**
 * @file IdleMemory.c
 * @author Ian Archbell
 * @brief FreeRTOS idle/timer task memory and failure hooks.
 *
 * Provides required weak implementations of:
 * - vApplicationGetIdleTaskMemory
 * - vApplicationGetTimerTaskMemory
 * - vApplicationStackOverflowHook
 * - vAssertCalled
 * - vApplicationMallocFailedHook
 *
 * These allow static allocation of idle and timer tasks and provide diagnostics for overflows and allocation failures.
 * Weak attributes allow user-defined implementations to override these if needed.
 *
 * @version 0.1
 * @date 2025-03-31
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

 #include "FreeRTOS.h"
 #include <stdio.h>
 #include <stdlib.h>
 #include <task.h>
 
 __attribute__((weak)) 
 void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                     StackType_t **ppxIdleTaskStackBuffer,
                                     uint32_t *pulIdleTaskStackSize )
 {
     static StaticTask_t xIdleTaskTCB;
     static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];
 
     *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
     *ppxIdleTaskStackBuffer = uxIdleTaskStack;
     *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
 }
 
 __attribute__((weak))
 void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
                                      StackType_t **ppxTimerTaskStackBuffer,
                                      uint32_t *pulTimerTaskStackSize )
 {
     static StaticTask_t xTimerTaskTCB;
     static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];
 
     *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
     *ppxTimerTaskStackBuffer = uxTimerTaskStack;
     *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
 }
 
 __attribute__((weak)) 
 void vApplicationStackOverflowHook( TaskHandle_t xTask,
                                     char *pcTaskName ){
     printf("----------------------------------------------\n");
     printf("STACK OVERFLOW on %s\n", pcTaskName);
     printf("----------------------------------------------\n");
     taskDISABLE_INTERRUPTS();
     for( ;; );
 }
 
 __attribute__((weak)) 
 void vAssertCalled( const char *pcFile, uint32_t ulLine ){
     printf("----------------------------------------------\n");
     printf("ASSERT FAILED %s line: %d\n", pcFile, ulLine);
     printf("----------------------------------------------\n");
     taskDISABLE_INTERRUPTS();
     for( ;; );
 }
 
 void __attribute__((weak)) vApplicationMallocFailedHook(void) {
     printf("\nMalloc failed!\n");
     printf(pcTaskGetName(NULL));
     printf("\n");
     exit(2);
 }
 