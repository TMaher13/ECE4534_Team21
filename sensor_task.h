/*
 *
 *
 *
 */

#ifndef SENSOR_TASK
#define SENSOR_TASK

#include <SimpleCodeStructure/sensor_thread_queue.h>
#include <SimpleCodeStructure/sensor_thread_state.h>

struct sensorQueueStruct {

    char *msg;

    int time;
    int dist;

    int error;

};


void *sensorThread(void *arg0);

int receiveMsg(void *rcv);


#include <SimpleCodeStructure/sensor_task.c>


#endif
