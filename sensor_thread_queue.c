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

/* RTOS header files */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

QueueHandle_t createSensorQueue(unsigned int queueLen, unsigned int itemSize) {

    return xQueueCreate(queueLen, itemSize);
}

int readSensorQueue(QueueHandle_t handle, void *data, bool blocking) {

    if(blocking) {

    }
    else {

    }

    return 0;
}

BaseType_t writeSensorQueue(QueueHandle_t handle, void *data, bool blocking) {

    BaseType_t ret;
    BaseType_t higherWoken;

    if(blocking) {
        ret = xQueueSend(handle, data, higherWoken);
    }
    else {
        ret = xQueueSendFromISR(handle, data, &higherWoken);
    }

    return ret;
}
