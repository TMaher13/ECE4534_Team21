/*
 *
 *
 *
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <queue_structs.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>


int sensorFSM(QueueHandle_t uart_handle, struct sensorQueueStruct *sensorMsg) {

    static int sensorTotal = 0, sensorCount = 0;
    static int fsmState = 0; // 0 for INIT_AVERAGE, 1 for UPDATE_AVERAGE

    char *uartMsg;
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

                uartMsg = malloc(sizeof(char) * 32);

                sprintf(uartMsg, "Avg = %dmm; Time = %dms\n", avg, sensorMsg->value);

                struct uartQueueStruct uart;
                uart.msg = uartMsg;

                // send to uart queue

                sensorTotal = 0;
                sensorCount = 0;
            }
            else if(sensorMsg->messageType == TIMER70_MESSAGE) {

                uartMsg = malloc(sizeof(char) * 32);

                sensorTotal += sensorMsg->value;
                sensorCount++;

                sprintf(uartMsg, "Sensor = %0.2f\n", sensorMsg->value);

                struct uartQueueStruct uart;
                uart.msg = uartMsg;

                // send to uart queue;

            }
            else
                return 2;
            break;
    }

    return 0;

}
