/*
 * uart_thread.h
 *
 *  Created on: Feb 8, 2021
 *      Author: Connor J. Bondi
 *
 * This task will receive messages
 * containing C-strings and output them to the UART.
 */

#ifndef UART_THREAD_H_
#define UART_THREAD_H_

#include <UART_thread_queue.h>

void *uart_task(void *argv);

int createUARTThread(int threadStackSize, int prio);

char *message[] sub_uart_recv()

int sub_uart_send(char *message[]);


#include <uart_thread.c>


#endif /* UART_THREAD_H_ */


