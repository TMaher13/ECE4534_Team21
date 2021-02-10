/*
 * uart_thread_queue.h
 *
 *  Created on: Feb 8, 2021
 *      Author: Connor J. Bondi
 *
 *  These files will be for the routines to create/read/write to/from each
 *  of  these message queues.
 */


#ifndef UART_THREAD_QUEUE_H_
#define UART_THREAD_QUEUE_H_

#include <stdbool.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include <queue_structs.h>

QueueHandle_t createUARTQueue(unsigned int queueLen, unsigned int itemSize);

int readUARTQueue(QueueHandle_t handle, void *data, bool blocking);

int writeUARTQueue(QueueHandle_t handle, void *data, bool blocking);

#include <uart_thread_queue.c>


#endif /* UART_THREAD_QUEUE_H_ */
