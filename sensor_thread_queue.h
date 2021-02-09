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

QueueHandle_t createSensorQueue(unsigned int queueLen, unsigned int itemSize);

int readSensorQueue(QueueHandle_t handle, void *data, bool blocking);

int writeSesnorQueue(QueueHandle_t handle, void *data, bool blocking);

#include <sensor_thread_queue.c>

#endif
