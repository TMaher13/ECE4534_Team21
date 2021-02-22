#ifndef SENSOR_THREAD_STATE
#define SENSOR_THREAD_STATE

//#include <queue_structs.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

int sensorFSM(QueueHandle_t uart_handle, struct sensorQueueStruct *sensorMsg);

#include <sensor_thread_state.c>

#endif
