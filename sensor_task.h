#ifndef SENSOR_TASK
#define SENSOR_TASK

void *sensorThread(void *arg0);

int createSensorThread(int threadStackSize, int prio);

#include <sensor_task.c>

#endif
