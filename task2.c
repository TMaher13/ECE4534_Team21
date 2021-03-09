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
#include <ti/drivers/Timer.h>
#include "timers.h"

#include <queue_structs.h>
#include <debug.h>
#include "debug_if.h"

extern BaseType_t readQueue(QueueHandle_t handle, const void * data);
extern BaseType_t writeQueue(QueueHandle_t handle, const void * data);


extern QueueHandle_t chain_handle;
extern QueueHandle_t publish_handle;

extern void dbgEvent(unsigned int event);
extern void fatalError(unsigned int event);

// FreeRTOS includes
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#define TIMER5000_PERIOD    5000

void task2Computation(char chainStr[SECRET_SIZE]) {
    int i=0;

    #if USER_ID == 0
        for(i=0; i<strlen(chainStr); i += 2) chainStr[i] = 'A';
        chainStr[0] = '0';

    #elif USER_ID == 1
        LOG_INFO("USER_ID1\r\n");
        i = 1;
        while(chainStr[i]!=NULL){
            chainStr[i] = 'B';
            i+=2;
        }
        chainStr[0] = '1';

    #elif USER_ID == 2
        for(i=1; i<strlen(chainStr); i += 2) chainStr[i] = 'C';
        chainStr[0] = '2';

    #elif USER_ID == 3
        for(i=1; i<strlen(chainStr); i += 2) chainStr[i] = 'D';
        chainStr[0] = '3';

    #endif
}

void timer5000Callback( TimerHandle_t xTimer )
{

    vTimerSetTimerID(xTimer, ( void * ) 5);

}

void *task2Thread(void *arg0) {

    //dbgEvent(ENTER_SENSOR_TASK);

    static struct chainQueueStruct chainData;

    TimerHandle_t timer5000 = xTimerCreate
            ( /* Just a text name, not used by the RTOS
              kernel. */
              "Timer5000",
              /* The timer period in ticks, must be
              greater than 0. */
              pdMS_TO_TICKS( TIMER5000_PERIOD ),
              /* The timers will auto-reload themselves
              when they expire. */
              pdFALSE,
              /* The ID is used to store a count of the
              number of times the timer has expired, which
              is initialised to 0. */
              ( void * ) 0,
              /* Each timer calls the same callback when
              it expires. */
              timer5000Callback
            );

    BaseType_t readRet;
    BaseType_t publishQueueRet;

    //create payload (JSON String)
    static struct publishQueueStruct publish;

    xTimerStart(timer5000,0);

    //dbgEvent(BEFORE_SENSOR_LOOP);

    for(;;) {

       // dbgEvent(BEFORE_READ_SENSOR_QUEUE);

        readRet = readQueue(chain_handle, &chainData);

        //dbgEvent(AFTER_READ_SENSOR_QUEUE);

        uint32_t ulCount = ( uint32_t ) pvTimerGetTimerID( timer5000 );

        if(readRet == pdTRUE) {

            task2Computation(chainData.secret);

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
            snprintf(publish.payload, SECRET_SIZE, "{\"secret\":\"%s\"}", chainData.secret); // chainData.secret

            publishQueueRet = writeQueue(publish_handle, &publish);

            xTimerReset(timer5000,0);
        }
        else if ((readRet == pdFALSE) && (ulCount == 5)){
            snprintf(publish.topic, TOPIC_SIZE, "chain1");
            memset(publish.payload, 0, SECRET_SIZE);
            snprintf(publish.payload, SECRET_SIZE, "{\"secret\":\"%s\"}", chainData.secret); // chainData.secret

            publishQueueRet = writeQueue(publish_handle, &publish);
            xTimerReset(timer5000,0);
        }

        if(ulCount == 5){
            vTimerSetTimerID(timer5000,( void * ) 0);
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
