/*
 *
 *
 *
 */

#ifndef SENSOR_TASK
#define SENSOR_TASK

#include <sensor_thread_queue.h>
#include <sensor_thread_state.h>

struct sensorQueueStruct {

    char *msg;

    int time;
    int dist;

    int error;

};

void *sensorThread(void *arg0);

int createSensorThread(int threadStackSize, int prio);

int receiveMsg(void *rcv);


#include <sensor_task.c>


#endif
