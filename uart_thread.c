/*
 * uart_thread.c
 *
 *  Created on: Feb 8, 2021
 *      Author: Connor J. Bondi
 *
 * This task will receive messages
 * containing C-strings and output them to the UART.
 */


/* General includes */
#include <unistd.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <debug.h>
/* POSIX Header files */
#include <pthread.h>

#include <queue_structs.h>

/* FreeRTOS includes */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

/* Driver Header files */
#include <ti/drivers/UART.h>

/* Driver configuration */
#include "ti_drivers_config.h"

extern QueueHandle_t uart_handle;

extern readUARTQueue(QueueHandle_t handle, struct uartQueueStruct *data);
extern writeUARTQueue(QueueHandle_t handle, struct uartQueueStruct *data);

extern void dbgEvent(unsigned int event);
extern void fatalError(unsigned int event);

int sub_uart_send(char *message){

    UART_init();

    UART_Handle uart_send;
    UART_Params uartParams;

    UART_Params_init(&uartParams);
    uartParams.writeMode = UART_MODE_BLOCKING;
    uartParams.readMode = UART_MODE_CALLBACK; // Does not block
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.baudRate = 115200;
    uartParams.readEcho = UART_ECHO_OFF;

    uart_send = UART_open(CONFIG_UART_0, &uartParams);
    if (uart_send == NULL) {
        fatalError(UART_INIT_FATAL_ERROR);
        return -1;
    }
    dbgEvent(BEFORE_WRITE_UART_QUEUE);
    UART_write(uart_send, (const void *)message, sizeof(message));
    dbgEvent(AFTER_WRITE_UART_QUEUE);
    return 1;

}


// Task used to receive strings and output them to UART (UART Send)
void *uart_task(void *arg0) {
    dbgEvent(ENTER_UART_TASK);
        /*
        You will have one task whose only job is to send to the UART. It won’t interface to any other
        I/O devices (and it won’t do UART receive). It can do some processing on data to be sent. It will
        read from a single FreeRTOS queue to get the data that needs to be sent.
        Body of your UART send task:
        */
    struct uartQueueStruct uartStruct;

    dbgEvent(BEFORE_UART_LOOP);
    while (1) {
        /* 1. Blocking receive call from a single FreeRTOS queue. */
            // THIS WILL BE A SUBROUTINE
        dbgEvent(BEFORE_READ_UART_QUEUE);
        while(readUARTQueue(uart_handle, &uartStruct) != pdTRUE) {
            // block until we read from queue
        }
        dbgEvent(AFTER_READ_UART_QUEUE);
        /* 2. Do any processing you want to do, but nothing else */

        /* 3. Blocking send call to TI UART driver */

            /* a. Make sure that you check for errors and halt if you get any */

            /* b. Send all of the data received from the queue */
        sub_uart_send(uartStruct.msg);

        continue;
    }

}


int createUARTThread(int threadStackSize, int prio) {

    pthread_t           thread;
    pthread_attr_t      attrs;
    struct sched_param  priParam;
    int                 retc;


    /* Initialize the attributes structure with default values */
    pthread_attr_init(&attrs);

    /* Set priority, detach state, and stack size attributes */
    priParam.sched_priority = prio;
    retc = pthread_attr_setschedparam(&attrs, &priParam);
    retc |= pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
    retc |= pthread_attr_setstacksize(&attrs, threadStackSize);
    if (retc != 0) {
        fatalError(UART_STACK_FATAL_ERROR);
        return -1; // Stack initialization failed
    }

    retc = pthread_create(&thread, &attrs, uart_task, NULL);
    if (retc != 0) {
        fatalError(UART_THREAD_FATAL_ERROR);
        return -2; // Thread/task creation failed
    }

    return 0;
}







// Task used to receive strings and output them to UART (UART Receive)
void *uart_recv_task(void *argv) {

        /*
        You will have one task whose only job is to receive from the UART. It won’t interface to any
        other I/O devices (and it won’t do UART send). It can do some processing on the received data.
        It will send the data via a FreeRTOS queue to whichever task(s) should receive it.
        Body of your UART receive task:
        */

    while (1) {
        /* 1. Blocking receive call to TI UART driver */

            /* a. Make sure that you check for errors and halt if you get any */

        /* 2. If needed, you can do some processing here, but nothing else. */

        /* 3. Send the data to one (or more) FreeRTOS queues using a blocking send. */

        continue;
    }

}

char* sub_uart_recv(){

    UART_init();

    UART_Handle uart_recv;
    UART_Params uartParams;
    unsigned char rxBuffer[20];

    UART_Params_init(&uartParams);
    uartParams.writeMode = UART_MODE_CALLBACK;
    uartParams.readMode = UART_MODE_CALLBACK; // Does not block
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.baudRate = 115200;
    uartParams.readEcho = UART_ECHO_OFF;

    uart_recv = UART_open(CONFIG_UART_0, &uartParams);
    if (uart_recv == NULL) {
        /* Error creating UART */
        return "-1";
    }

    UART_read(uart_recv, rxBuffer, sizeof(rxBuffer));

    return rxBuffer;

}

