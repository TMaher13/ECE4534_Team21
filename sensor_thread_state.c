#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <queue_structs.h>
#include <debug.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

extern writeQueue(QueueHandle_t handle, void * const data);

extern void dbgEvent(unsigned int event);
extern void fatalError(unsigned int event);

int sensorFSM(QueueHandle_t publish_handle, struct sensorQueueStruct *sensorMsg) {

    static int sensorTotal = 0, sensorCount = 0;
    static int fsmState = 0; // 0 for INIT_AVERAGE, 1 for UPDATE_AVERAGE

    BaseType_t publishQueueRet;

    //create payload (JSON String)
    static struct publishQueueStruct publish;
    char jsonStr[PAYLOAD_SIZE];

    int avg;

    static int publishAttempts = 0;

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

                //set topic
                snprintf(publish.topic, TOPIC_SIZE, "joseph_sensor");

                //set payload
                memset(jsonStr, 0, PAYLOAD_SIZE);
                snprintf(jsonStr, PAYLOAD_SIZE, "{\"messageType\":\"%d\",\"messageID\":\"%d\",\"value1\":\"%d\",\"value2\":\"%d\"}", TIMER500_MESSAGE, publishAttempts, avg, sensorMsg->value);
                memcpy(publish.payload,jsonStr, PAYLOAD_SIZE);

                publishQueueRet = writeQueue(publish_handle, &publish);

                publishAttempts++;

                //set topic
                snprintf(publish.topic, TOPIC_SIZE, "connorStat");

                //set payload
                memset(jsonStr, 0, PAYLOAD_SIZE);
                snprintf(jsonStr, PAYLOAD_SIZE, "{\"publishAttempts\":\"%d\"}", publishAttempts);
                memcpy(publish.payload,jsonStr, PAYLOAD_SIZE);

                publishQueueRet = writeQueue(publish_handle, &publish);

                sensorTotal = 0;
                sensorCount = 0;
                fsmState = 0;
            }
            else if(sensorMsg->messageType == TIMER70_MESSAGE) {

                sensorTotal += sensorMsg->value;
                sensorCount++;

                /*
                memset(uartMsg, 0, PAYLOAD_SIZE);
                snprintf(uartMsg, PAYLOAD_SIZE, "Sensor %d = %dmm", sensorCount, sensorMsg->value);
                memcpy(uart.msg,uartMsg, PAYLOAD_SIZE);

                dbgEvent(BEFORE_WRITE_UART_QUEUE_TIMER70);
                uartQueueRet = writeUARTQueue(uart_handle, &uart);
                dbgEvent(AFTER_WRITE_UART_QUEUE_TIMER70);
                if(uartQueueRet != pdPASS) {
                    fatalError(WRITE_UART_QUEUE_FATAL_ERROR_TIMER70);
                }
                */

                //set topic
                snprintf(publish.topic, TOPIC_SIZE, "joseph_sensor");

                //set payload
                memset(jsonStr, 0, PAYLOAD_SIZE);
                snprintf(jsonStr, PAYLOAD_SIZE, "{\"messageType\":\"%d\",\"messageID\":\"%d\",\"value1\":\"%d\",\"value2\":\"%d\"}", TIMER70_MESSAGE, publishAttempts, sensorCount, sensorMsg->value);
                memcpy(publish.payload,jsonStr, PAYLOAD_SIZE);
                publishQueueRet = writeQueue(publish_handle, &publish);

                publishAttempts++;

                //set topic
                snprintf(publish.topic, TOPIC_SIZE, "connorStat");

                //set payload
                memset(jsonStr, 0, PAYLOAD_SIZE);
                snprintf(jsonStr, PAYLOAD_SIZE, "{\"publishAttempts\":\"%d\"}", publishAttempts);
                memcpy(publish.payload,jsonStr, PAYLOAD_SIZE);

                publishQueueRet = writeQueue(publish_handle, &publish);

            }
            else
                return 2;
            break;
    }

    return 0;

}
