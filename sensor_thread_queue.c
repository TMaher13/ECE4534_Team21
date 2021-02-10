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

#include <queue_structs.h>

QueueHandle_t createSensorQueue(unsigned int queueLen, unsigned int itemSize) {

    return xQueueCreate(queueLen, itemSize);
}

BaseType_t readSensorQueue(QueueHandle_t handle, struct sensorQueueStruct *data) {

    return xQueueReceive(handle, data, 100);
}

BaseType_t writeSensorQueue(QueueHandle_t handle, struct sensorQueueStruct *data) {

    return xQueueSend(handle, data, 100);
}
