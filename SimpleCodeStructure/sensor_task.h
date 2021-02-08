/*
 *
 *
 *
 */

#ifndef SENSOR_TASK
#define SENSOR_TASK

#include <sensor_thread_queue.h>
#include <sensor_thread_state.h>


void *sensorThread(void *arg0);

int receiveMsg(sensorQueueStruct *rcv);


#include <sensor_task.c>


#define
