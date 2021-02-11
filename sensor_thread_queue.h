/*
 *
 *
 *
 */

#ifndef SENSOR_THREAD_QUEUE
#define SENSOR_THREAD_QUEUE

#include <stdbool.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include <queue_structs.h>


QueueHandle_t createSensorQueue(unsigned int queueLen, unsigned int itemSize);

BaseType_t readSensorQueue(QueueHandle_t handle, struct sensorQueueStruct *data);

BaseType_t writeSesnorQueue(QueueHandle_t handle, struct sensorQueueStruct *data);

BaseType_t writeSensorQueueCallback(const void *pvItemToQueue);

#endif
