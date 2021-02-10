/*
 * Copyright (c) 2016-2020, Texas Instruments Incorporated
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

#ifdef __ICCARM__
#include <DLib_Threads.h>
#endif

/* RTOS header files */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include <ti/drivers/GPIO.h>
#include <ti_drivers_config.h>
#include <ti/drivers/Board.h>

#include <queue_structs.h>

extern int createSensorThread(QueueHandle_t sensor_handle, QueueHandle_t uart_handle, int threadStackSize, int prio);

extern QueueHandle_t createSensorQueue(unsigned int queueLen, unsigned int itemSize);

/* Stack size in bytes */
#define THREADSTACKSIZE   1024

/*
 *  ======== main ========
 */
int main(void) {

    /* initialize the system locks */
#ifdef __ICCARM__
    __iar_Initlocks();
#endif

    Board_init();

    QueueHandle_t sensor_handle = createSensorQueue(10, sizeof(struct sensorQueueStruct));
    //QueueHandle_t uart_handle = createUartQueue(10, sizeof(struct uartQueueStruct));
    QueueHandle_t uart_handle;

    if(sensor_handle == NULL)
        return (1);

    /*if(!createSensorThread(sensor_handle, THREADSTACKSIZE, 1)) {
        GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);
    }*/
    createSensorThread(sensor_handle, uart_handle, THREADSTACKSIZE, 1);

    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();

    return (0);
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
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
    //Handle FreeRTOS Stack Overflow
    while(1)
    {
    }
}
