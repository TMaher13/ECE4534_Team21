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

char *message[] sub_uart_recv();

void *uart_recv_task(void *argv);

int sub_uart_send(char *message, UART_Handle uart_send);


#include <uart_thread.c>


#endif /* UART_THREAD_H_ */


