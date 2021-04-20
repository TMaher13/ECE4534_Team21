#ifndef SENSOR_THREAD_QUEUE
#define SENSOR_THREAD_QUEUE

#include <stdbool.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include <queue_structs.h>


QueueHandle_t createQueue(unsigned int queueLen, unsigned int itemSize);

BaseType_t readQueue(QueueHandle_t handle, const void *data);

BaseType_t writeQueue(QueueHandle_t handle, const void *data);

BaseType_t writeQueueCallback(QueueHandle_t handle, const void *m);

#endif
