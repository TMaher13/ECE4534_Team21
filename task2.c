#include <unistd.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <stdio.h>

/* POSIX Header files */
#include <pthread.h>

/* RTOS header files */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include "uart_term.h"

#include "user_def.h"

#include <ti/drivers/GPIO.h>
#include <ti_drivers_config.h>

#include <queue_structs.h>
#include <debug.h>

extern BaseType_t readQueue(QueueHandle_t handle, void * const data);
extern BaseType_t writeQueue(QueueHandle_t handle, const void * data);

extern QueueHandle_t chain_handle;
extern QueueHandle_t publish_handle;

extern void dbgEvent(unsigned int event);
extern void fatalError(unsigned int event);

// FreeRTOS includes
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>


void task2Computation(char chainStr[SECRET_SIZE]) {
    int i;

#if USER_ID == 0
    for(i=0; i<strlen(chainStr); i += 2) chainStr[i] = 'A';
    chainStr[0] = '0';

#elif USER_ID == 1
    for(i=1; i<strlen(chainStr); i += 2) chainStr[i] = 'B';
    chainStr[0] = '1';

#elif USER_ID == 2
    for(i=0; i<strlen(chainStr); i += 2) chainStr[i] = 'C';
    chainStr[0] = '2';

#elif USER_ID == 3
    for(i=1; i<strlen(chainStr); i += 2) chainStr[i] = 'D';
    chainStr[0] = '3';

#endif
}


void *task2Thread(void *arg0) {

    //dbgEvent(ENTER_SENSOR_TASK);

    struct chainQueueStruct chainData;
    BaseType_t readRet;
    BaseType_t publishQueueRet;

    //create payload (JSON String)
    static struct publishQueueStruct publish;

    //dbgEvent(BEFORE_SENSOR_LOOP);

    for(;;) {

       // dbgEvent(BEFORE_READ_SENSOR_QUEUE);

        readRet = readQueue(chain_handle, &chainData);

        //dbgEvent(AFTER_READ_SENSOR_QUEUE);

        if(readRet == pdTRUE) {

            UART_PRINT(chainData.secret);
            task2Computation(chainData.secret);
            UART_PRINT(chainData.secret);

#if USER_ID == 0
            snprintf(publish.topic, TOPIC_SIZE, "chain0");
#elif USER_ID == 1
            snprintf(publish.topic, TOPIC_SIZE, "chain1");
#elif USER_ID == 2
            snprintf(publish.topic, TOPIC_SIZE, "chain2");
#elif USER_ID == 3
            snprintf(publish.topic, TOPIC_SIZE, "chain3");
#endif
            //set payload
            memset(publish.payload, 0, SECRET_SIZE);
            snprintf(publish.payload, SECRET_SIZE, "{\"secret\":\"%s\"}", chainData.secret);

            publishQueueRet = writeQueue(publish_handle, &publish);
        }

    }
}


int createTask2Thread(int threadStackSize, int prio) {

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
        //fatalError(SENSOR_STACK_FATAL_ERROR);
        return -1; // Stack initialization failed
    }

    retc = pthread_create(&thread, &attrs, task2Thread, NULL);
    if (retc != 0) {
        //fatalError(SENSOR_THREAD_FATAL_ERROR);
        return -2; // Thread/task creation failed
    }

    return 0;
}

/*
            jsmn_parser_parser;
            jsmn_init(&parser);
            jsmntok_t tokens[256];
            int r;
            r = jsmn_parse(&parser, data, dataLen)
            receivedMetaData = (MQTTClient_RecvMetaDataCB *)metaData;
            */
