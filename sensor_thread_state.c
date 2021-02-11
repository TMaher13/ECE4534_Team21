/*
 *
 *
 *
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <queue_structs.h>
#include <debug.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

extern writeUARTQueue(QueueHandle_t handle, struct uartQueueStruct **data);

extern void dbgEvent(unsigned int event);
extern void fatalError(unsigned int event);

int sensorFSM(QueueHandle_t uart_handle, struct sensorQueueStruct *sensorMsg) {

    static int sensorTotal = 0, sensorCount = 0;
    static int fsmState = 0; // 0 for INIT_AVERAGE, 1 for UPDATE_AVERAGE

    BaseType_t uartQueueRet;

    struct uartQueueStruct *uart =  malloc(sizeof(struct uartQueueStruct));
    char uartMsg[BUFFER_SIZE];

    double avg;

    switch(fsmState) {

        case 0: // INIT_AVERAGE
            if(sensorMsg->messageType == TIMER500_MESSAGE)
                fsmState = 1;
            else if(sensorMsg->messageType == TIMER70_MESSAGE)
                fsmState = 0;
            else
                return 1;
            break;

        case 1: // UPDATE_AVERAGE
            if(sensorMsg->messageType == TIMER500_MESSAGE) {

                if(sensorCount != 0)
                    avg = sensorTotal / sensorCount;
                else
                    avg = 0.0;

                memset(uartMsg, 0, BUFFER_SIZE);
                snprintf(uartMsg, BUFFER_SIZE, "Avg = %0.2fmm; Time = %dms\n", avg, sensorMsg->value);
                memcpy(uart->msg,uartMsg, BUFFER_SIZE);

                dbgEvent(BEFORE_WRITE_UART_QUEUE_TIMER500);
                uartQueueRet = writeUARTQueue(uart_handle, &uart);
                dbgEvent(AFTER_WRITE_UART_QUEUE_TIMER500);
                if(uartQueueRet != pdPASS) {
                    fatalError(WRITE_UART_QUEUE_FATAL_ERROR_TIMER500);
                }

                sensorTotal = 0;
                sensorCount = 0;
                fsmState = 0;
            }
            else if(sensorMsg->messageType == TIMER70_MESSAGE) {

                //uartMsg = malloc(sizeof(char) * 32);

                sensorTotal += sensorMsg->value;
                sensorCount++;

                /*
                sprintf(uartMsg, "Sensor %d = %dmm\n", sensorCount, sensorMsg->value);
                (*uart).msg = uartMsg;

                dbgEvent(BEFORE_WRITE_UART_QUEUE_TIMER70);
                uartQueueRet = writeUARTQueue(uart_handle, &uart);
                dbgEvent(AFTER_WRITE_UART_QUEUE_TIMER70);
                if(uartQueueRet != pdPASS) {
                    fatalError(WRITE_UART_QUEUE_FATAL_ERROR_TIMER70);
                }
                */

            }
            else
                return 2;
            break;
    }

    return 0;

}
