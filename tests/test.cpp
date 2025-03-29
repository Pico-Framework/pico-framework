#include "pico/stdlib.h"
#include <stdio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "test.hpp"
#include "logger.hpp"
#include "controller.hpp"

// Priorities of our thread - higher numbers are higher priority
#define TASK_PRIORITY     1

// Stack sizes of our threads in words
#define TASK_STACK_SIZE 1024

    /**
     * @brief Launches test task.
     * 
     * This function is used to get around the fact xTaskCreate() from the freertos api requires a static task function.
     * 
     * To prevent having to write the test task from the context of a static function, this launches the test_task()
     * from the Test object passed into xTaskCreate().
     * 
     * We could also use the test instance array to do this
     * 
     * @param arg a pointer to the button to detect events for casted to a void pointer
     * @return void, nothing to return
     */
    void Test::test_task_trampoline(void *arg){
        Test *local_test = (Test *)arg; //cast to button pointer
        printf("In trampoline\n");
        local_test->vTestTask(); //launch test task
    }

    /*
    * The definition of the test task itself.  
    */
    void Test::vTestTask()
    {
        // do tests
        printf("Running tests\n");
//        testLogger();
          testController();

        for( ;; )
        {
            //printf("In task loop\n");
            /* Block on the notify to wait for an interrupt event.  The snotification
            is 'given' from button_handler() below.  Using portMAX_DELAY as the
            block time will cause the task to block indefinitely provided
            INCLUDE_vTaskSuspend is set to 1 in FreeRTOSConfig.h. */
            
            // do nothing - could delete task
            vTaskDelay(3000);
        }      
    }
 
     Test::Test(){
            xTaskCreate(&test_task_trampoline, "test_task", TASK_STACK_SIZE, this, TASK_PRIORITY, &m_test_task_hdl);
    }

    // void Test::testLogger(){
    //     Logger slog;
    //     int age = 32;
    //     slog << "Hello, I am " << age << " years old" << std::endl;
    //     slog << "That's " << std::hex << age << " years in hex" << std::endl;
    //     slog(Logger::ERROR) << "Now I'm logging an error" << std::endl;
    //     slog << "However, after a flush/endl, the error will revert to INFO" << std::endl << std::ends;
    //     slog(Logger::WARN) << "Hello, I am " << age << " years old" << std::endl << std::ends;
    // }

    void Test::testController(){
        // Controller controller;
        // controller.run_zone("Raspberries");
        // vTaskDelay(3000);
        // controller.stop_zone("Raspberries");
        // vTaskDelay(1000);
        // //controller.run_zone("Rhubarb");
        // vTaskDelay(3000);
        // //controller.stop_zone("Rhubarb");
    }


    
