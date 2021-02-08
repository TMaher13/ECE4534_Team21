/*
 *
 *
 *
 */

#ifndef SENSOR_TASK
#define SENSOR_TASK

struct sensorStruct {

    char *msg;

    int error;

};


void *sensorThread(void *arg0);

int receiveMsg(sensorStruct *rcv);


#include <sensor_task.c>


#define
