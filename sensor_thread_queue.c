#include <unistd.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/* RTOS header files */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <debug.h>
//#include "sensor_thread_queue.h"

extern QueueHandle_t sensor_handle;
extern QueueHandle_t chain_handle;

extern void dbgEvent(unsigned int event);

#include <queue_structs.h>

QueueHandle_t createSensorQueue(unsigned int queueLen, unsigned int itemSize) {
    return xQueueCreate(queueLen, itemSize);
}

QueueHandle_t createQueue(unsigned int queueLen, unsigned int itemSize) {
    return xQueueCreate(queueLen, itemSize);
}

BaseType_t readSensorQueue(QueueHandle_t handle, struct sensorQueueStruct *data) {

    return xQueueReceive(handle, data, 100);
}

BaseType_t writeSensorQueue(QueueHandle_t handle, struct sensorQueueStruct *data) {

    return xQueueSend(handle, data, 100);
}

BaseType_t readQueue(QueueHandle_t handle, void * const data) {

    return xQueueReceive(handle, data, 100);
}

BaseType_t writeQueue(QueueHandle_t handle, const void * data) {

    return xQueueSend(handle, data, 100);
}

BaseType_t writeSensorQueueCallback(struct sensorQueueStruct *m)
{
    dbgEvent(ENTER_SENSOR_QUEUE_CALLBACK);
    BaseType_t res, xHigherPriorityTaskWoken;

    xHigherPriorityTaskWoken = pdFALSE;

    res = xQueueSendFromISR(sensor_handle, m, &xHigherPriorityTaskWoken);


    if( xHigherPriorityTaskWoken ) {
        /* Actual macro used here is port specific. */
        taskYIELD ();
    }

    dbgEvent(LEAVE_SENSOR_QUEUE_CALLBACK);
    return res;
}

BaseType_t writeChainQueueCallback(const void *m)
{
    //dbgEvent(ENTER_SENSOR_QUEUE_CALLBACK);
    BaseType_t res, xHigherPriorityTaskWoken;

    xHigherPriorityTaskWoken = pdFALSE;

    res = xQueueSendFromISR(chain_handle, m, &xHigherPriorityTaskWoken);


    if( xHigherPriorityTaskWoken ) {
        /* Actual macro used here is port specific. */
        taskYIELD ();
    }

    //dbgEvent(LEAVE_SENSOR_QUEUE_CALLBACK);
    return res;
}
