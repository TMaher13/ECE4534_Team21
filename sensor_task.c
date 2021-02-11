/*
 *
 *
 *
 */

// General includes
#include <unistd.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

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

extern BaseType_t readSensorQueue(QueueHandle_t handle, struct sensorQueueStruct *data);
extern BaseType_t writeSensorQueue(QueueHandle_t handle, struct sensorQueueStruct *data, bool blocking);

extern int sensorFSM(QueueHandle_t uart_handle, struct sensorQueueStruct *sensorMsg);

extern QueueHandle_t sensor_handle;

extern void dbgEvent(unsigned int event);
extern void fatalError(unsigned int event);

// FreeRTOS includes
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>


void *sensorThread(void *arg0) {

    dbgEvent(ENTER_SENSOR_TASK);

    struct sensorQueueStruct sensorData; // = {TIMER70_MESSAGE, 0};
    int fsm_ret = 0;

    dbgEvent(BEFORE_SENSOR_LOOP);

    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    for(;;) {

        dbgEvent(BEFORE_READ_SENSOR_QUEUE);

        readSensorQueue( sensor_handle, &sensorData);
        while( sensorData.messageType != TIMER70_MESSAGE && sensorData.messageType != TIMER500_MESSAGE) {
            // block and wait to read
            readSensorQueue( sensor_handle, &sensorData);
        }

        dbgEvent(AFTER_READ_SENSOR_QUEUE);

        fsm_ret = sensorFSM( sensor_handle, &sensorData );
        if(fsm_ret == 1){
            fatalError(FSM_FATAL_ERROR1);
            return NULL;
        }
        else if(fsm_ret == 2){
            fatalError(FSM_FATAL_ERROR2);
            return NULL;
        }
    }

    return NULL;
}


int createSensorThread(int threadStackSize, int prio) {

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
        fatalError(SENSOR_STACK_FATAL_ERROR);
        return -1; // Stack initialization failed
    }

    retc = pthread_create(&thread, &attrs, sensorThread, NULL);
    if (retc != 0) {
        fatalError(SENSOR_THREAD_FATAL_ERROR);
        return -2; // Thread/task creation failed
    }

    return 0;
}
