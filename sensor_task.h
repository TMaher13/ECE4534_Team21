/*
 *
 *
 *
 */

#ifndef SENSOR_TASK
#define SENSOR_TASK

void *sensorThread(void *arg0);

int createSensorThread(QueueHandle_t sensor_handle, QueueHandle_t uart_handle, int threadStackSize, int prio);

#include <sensor_task.c>


#endif
