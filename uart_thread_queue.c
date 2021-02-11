/*
 * uart_thread_queue.c
 *
 *  Created on: Feb 8, 2021
 *      Author: Connor J. Bondi
 *
 *  These files will be for the routines to create/read/write to/from each
 *  of  these message queues.
 */



// General includes
#include <unistd.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/* RTOS header files */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include <queue_structs.h>

QueueHandle_t createUARTQueue(unsigned int queueLen, unsigned int itemSize) {

    return xQueueCreate(queueLen, itemSize);

}

BaseType_t readUARTQueue(QueueHandle_t handle, struct uartQueueStruct *data) {

    return xQueueReceive(handle, data, 100);

}

BaseType_t writeUARTQueue(QueueHandle_t handle, struct uartQueueStruct *data) {

    return xQueueSend(handle, data, 100);

}
