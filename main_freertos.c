/*
 * Copyright (c) 2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== main_freertos.c ========
 */
#include <stdint.h>

/* POSIX Header files */
#include <pthread.h>

/* RTOS header files */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/I2C.h>

/* TI-DRIVERS Header files */
#include "ti_drivers_config.h"

#include <queue_structs.h>

extern void * mqttThread(void *arg0);

/* Stack size in bytes */
#define THREADSTACKSIZE   2048

extern void timer500Init();
extern void timer70Init();
extern void timer1000Init();
extern void debugInit();

extern QueueHandle_t createSensorQueue(unsigned int queueLen, unsigned int itemSize);
extern QueueHandle_t createQueue(unsigned int queueLen, unsigned int itemSize);

extern int createLidarThread(int threadStackSize, int prio);
extern int createCameraThread(int threadStackSize, int prio);
extern int createNavigationThread(int threadStackSize, int prio);
extern int createMQTTThread(int threadStackSize, int prio);


// Task queue handles
QueueHandle_t lidar_handle;
//QueueHandle_t camera_handle;
QueueHandle_t nav_handle;

// MQTT task handle
QueueHandle_t mqtt_handle;

/*
 *  ======== main ========
 */
int main(void) {

    /* Call board init functions */
    Board_init();
    debugInit();

    // Initialize I2C for interfacing with OpenMV H7 camera
    I2C_init();

    // Initialize UART and GPIO for interfacing with RPLidar A1 sensor
    UART_init();
    GPIO_init();
    //GPIO_setConfig(CONFIG_GPIO_MOTOCTL_8, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF);

    // Queue for sending navigation data to the nvagigation task
    lidar_handle = createQueue(5, sizeof(struct lidarQueueStruct));
    if (lidar_handle == NULL) {
        GPIO_toggle(CONFIG_GPIO_LED_0);
        return 1;
    }

    // Queue for sending navigation data to the nvagigation task
    nav_handle = createQueue(5, sizeof(struct navQueueStruct));
    if (nav_handle == NULL) {
        GPIO_toggle(CONFIG_GPIO_LED_0);
        return 1;
    }

    // Queue for publishing messages to MQTT
    mqtt_handle = createQueue(5, sizeof(struct mqttQueueStruct));
    if(mqtt_handle == NULL) {
        GPIO_toggle(CONFIG_GPIO_LED_0);
        return 1;
    }

    createMQTTThread(THREADSTACKSIZE, 1);

    createLidarThread(THREADSTACKSIZE, 1);

    //createCameraThread(THREADSTACKSIZE, 1);

    //createNavigationThread(THREADSTACKSIZE, 1);

    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();

    return (0);
}

//*****************************************************************************
//
//! \brief Application defined malloc failed hook
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void vApplicationMallocFailedHook()
{
    /* Handle Memory Allocation Errors */
    while(1)
    {
    }
}

//*****************************************************************************
//
//! \brief Application defined stack overflow hook
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void vApplicationStackOverflowHook(TaskHandle_t pxTask,
                                   char *pcTaskName)
{
    //Handle FreeRTOS Stack Overflow
    while(1)
    {
    }
}

void vApplicationTickHook(void)
{
    /*
     * This function will be called by each tick interrupt if
     * configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
     * added here, but the tick hook is called from an interrupt context, so
     * code must not attempt to block, and only the interrupt safe FreeRTOS API
     * functions can be used (those that end in FromISR()).
     */
}

void vPreSleepProcessing(uint32_t ulExpectedIdleTime)
{
}

//*****************************************************************************
//
//! \brief Application defined idle task hook
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void
vApplicationIdleHook(void)
{
    /* Handle Idle Hook for Profiling, Power Management etc */
}

//*****************************************************************************
//
//! \brief  Overwrite the GCC _sbrk function which check the heap limit related
//!         to the stack pointer.
//!         In case of freertos this checking will fail.
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
#if defined (__GNUC__)
void * _sbrk(uint32_t delta)
{
    extern char _end;     /* Defined by the linker */
    extern char __HeapLimit;
    static char *heap_end;
    static char *heap_limit;
    char *prev_heap_end;

    if(heap_end == 0)
    {
        heap_end = &_end;
        heap_limit = &__HeapLimit;
    }

    prev_heap_end = heap_end;
    if(prev_heap_end + delta > heap_limit)
    {
        return((void *) -1L);
    }
    heap_end += delta;
    return((void *) prev_heap_end);
}

#endif
