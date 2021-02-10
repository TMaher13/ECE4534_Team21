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
//#include <sensor_thread_queue.h>
//#include <sensor_thread_state.h>

extern BaseType_t readSensorQueue(QueueHandle_t handle, struct sensorQueueStruct *data);
extern BaseType_t writeSesnorQueue(QueueHandle_t handle, struct sensorQueueStruct *data, bool blocking);

extern int sensorFSM(QueueHandle_t uart_handle, struct sensorQueueStruct *sensorMsg);

// FreeRTOS includes
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

struct QueueHandles {
    QueueHandle_t sensor_handle;
    QueueHandle_t uart_handle;
};


void *sensorThread(void *arg0) {

#if 0
    // For light debugging
    //sensorStruct myStruct;
    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    /* Turn off user LED */
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF);

    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);

    for(;;) {
        GPIO_toggle(CONFIG_GPIO_LED_0);
        vTaskDelay(1000);
    }

    return NULL;

#else

    struct sensorQueueStruct *sensorData;
    int fsm_ret = 0;

    for(;;) {

        if(readSensorQueue( ((struct QueueHandles*)arg0)->sensor_handle, sensorData) == pdFALSE)
            // Return error
            break;

        fsm_ret = sensorFSM( ((struct QueueHandles*)arg0)->uart_handle, sensorData );
        if(fsm_ret == 1)
            // error
            return NULL;
        else if(fsm_ret == 2)
            // error
            return NULL;
    }

    return NULL;
#endif
}

int createSensorThread(QueueHandle_t sensor_handle, QueueHandle_t uart_handle, int threadStackSize, int prio) {

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
        return -1; // Stack initialization failed
    }

    struct QueueHandles queueHandles = {sensor_handle, uart_handle};

    retc = pthread_create(&thread, &attrs, sensorThread, &queueHandles);
    if (retc != 0) {
        return -2; // Thread/task creation failed
    }

    return 0;
}
