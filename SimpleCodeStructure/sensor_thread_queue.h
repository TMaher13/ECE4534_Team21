/*
 *
 *
 *
 */

#ifndef SENSOR_THREAD_QUEUE
#define SENSOR_THREAD_QUEUE

struct sensorQueueStruct {

    char *msg;

    int time;
    int dist;

    int error;

};

int createSensorQueue();

int readSensorQueue();

int writeSesnorQueue();

#include <sensor_thread_queue.c>

#endif
