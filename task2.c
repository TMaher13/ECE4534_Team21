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


void *task2Thread(void *arg0) {

    //dbgEvent(ENTER_SENSOR_TASK);

    struct chainQueueStruct chainData;
    BaseType_t readRet;
    BaseType_t publishQueueRet;

    //create payload (JSON String)
    static struct publishQueueStruct publish;
    char jsonStr[PAYLOAD_SIZE];

    //dbgEvent(BEFORE_SENSOR_LOOP);

    for(;;) {

       // dbgEvent(BEFORE_READ_SENSOR_QUEUE);

        readRet = readQueue(chain_handle, &chainData);

        //dbgEvent(AFTER_READ_SENSOR_QUEUE);

        if(readRet == pdTRUE) {

            //set topic
            snprintf(publish.topic, TOPIC_SIZE, "chain2");

            //set payload
            memset(jsonStr, 0, PAYLOAD_SIZE);
            snprintf(jsonStr, PAYLOAD_SIZE, chainData.secret);
            memcpy(publish.payload,jsonStr, PAYLOAD_SIZE);

            publishQueueRet = writeQueue(publish_handle, &publish);

            /*
            if(fsm_ret == 1){
                fatalError(FSM_FATAL_ERROR1);
                return NULL;
            }
            else if(fsm_ret == 2){
                fatalError(FSM_FATAL_ERROR2);
                return NULL;
            }
            */
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
