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
//#include <sensor_thread_queue.h>
//#include <sensor_thread_state.h>

extern BaseType_t readQueue(QueueHandle_t handle, void * const data);
extern BaseType_t writeQueue(QueueHandle_t handle, void * const data);

extern QueueHandle_t receive_handle;
extern QueueHandle_t publish_handle;

extern void dbgEvent(unsigned int event);
extern void fatalError(unsigned int event);

// FreeRTOS includes
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>


void *receiveThread(void *arg0) {

    //dbgEvent(ENTER_SENSOR_TASK);

    struct receiveQueueStruct receiveData;
    BaseType_t readRet;

    //dbgEvent(BEFORE_SENSOR_LOOP);

    static int publishReceived = 0;
    static int publishAttempts = 0;

    static int totalMessages = 0;
    static int totalReadings = 0;
    static int readingsAvgTotal = 0;
    static int avgCount = 0;

    static int expectedMessageID = 0;
    static int totalLostMessages = 1;

    BaseType_t publishQueueRet;

    //create payload (JSON String)
    static struct publishQueueStruct publish;
    char jsonStr[PAYLOAD_SIZE];

    for(;;) {

        //dbgEvent(BEFORE_READ_SENSOR_QUEUE);

        readRet = readQueue( receive_handle, &receiveData);

        //dbgEvent(AFTER_READ_SENSOR_QUEUE);

        if(readRet == pdTRUE) {

            if (receiveData.messageID != expectedMessageID){
                //set topic
                snprintf(publish.topic, TOPIC_SIZE, "lostMessages");

                //set payload
                memset(jsonStr, 0, PAYLOAD_SIZE);
                snprintf(jsonStr, PAYLOAD_SIZE, "{\"ExpectedMessageID\":\"%d\",\"ReceivedMessageID\":\"%d\",\"totalLostMessages\":\"%d\"}", expectedMessageID, receiveData.messageID, totalLostMessages);
                memcpy(publish.payload,jsonStr, PAYLOAD_SIZE);

                //send to publish queue
                publishQueueRet = writeQueue(publish_handle, &publish);
                if (publishQueueRet == 0){
                //DEBUG_EVENT
                }

                totalLostMessages++;
            }

            expectedMessageID = receiveData.messageID + 1;

            publishReceived++;
            if (receiveData.messageType == TIMER70_MESSAGE){
                totalMessages++;
                totalReadings = totalReadings + receiveData.value1;
            }
            else if (receiveData.messageType == TIMER500_MESSAGE){
                totalMessages++;
                avgCount++;
                readingsAvgTotal = readingsAvgTotal + receiveData.value1;
            }
            else if (receiveData.messageType == TIMER1000_MESSAGE){
                int avg = readingsAvgTotal / avgCount;
                int numMessages = totalReadings;

                //set topic
                snprintf(publish.topic, TOPIC_SIZE, "kevin");

                //set payload
                memset(jsonStr, 0, PAYLOAD_SIZE);
                snprintf(jsonStr, PAYLOAD_SIZE, "{\"messageCounter\":\"%d\",\"totalMessages\":\"%d\",\"totalReadings\":\"%d\",\"avgReadings\":\"%d\"}", publishReceived, totalMessages, numMessages, avg);
                memcpy(publish.payload,jsonStr, PAYLOAD_SIZE);

                //send to publish queue
                publishQueueRet = writeQueue(publish_handle, &publish);
                if (publishQueueRet == 0){
                    //DEBUG_EVENT
                }
                publishAttempts++;

                //reset counts every publish
                readingsAvgTotal = 0;
                avgCount = 0;
                totalMessages = 0;
                totalReadings = 0;

                }
                else{
                    snprintf(publish.topic, TOPIC_SIZE, "badPayload");

                    //set payload
                    memset(jsonStr, 0, PAYLOAD_SIZE);
                    snprintf(jsonStr, PAYLOAD_SIZE, "{\"value1\":\"%d\",\"value2\":\"%d\"}", receiveData.value1, receiveData.value2);
                    memcpy(publish.payload,jsonStr, PAYLOAD_SIZE);

                    //send to publish queue
                    publishQueueRet = writeQueue(publish_handle, &publish);
                    if (publishQueueRet == 0){
                        //DEBUG_EVENT
                    }
                }

                //set topic
                snprintf(publish.topic, TOPIC_SIZE, "connorStat");

                //set payload
                memset(jsonStr, 0, PAYLOAD_SIZE);
                snprintf(jsonStr, PAYLOAD_SIZE,
                         "{\"publishReceived\":\"%d\",\"publishAttempts\":\"%d\"}",
                         publishReceived, publishAttempts);
                memcpy(publish.payload, jsonStr, PAYLOAD_SIZE);

                //send to publish queue
                publishQueueRet = writeQueue(publish_handle, &publish);
                if (publishQueueRet == 0)
                {
                    //DEBUG_EVENT
                }
            }
    }
}


int createReceiveThread(int threadStackSize, int prio) {

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
        //fatalError(RECEIVE_STACK_FATAL_ERROR);
        return -1; // Stack initialization failed
    }

    retc = pthread_create(&thread, &attrs, receiveThread, NULL);
    if (retc != 0) {
        //fatalError(RECEIVE_THREAD_FATAL_ERROR);
        return -2; // Thread/task creation failed
    }

    return 0;
}
