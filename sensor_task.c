/*
 *
 *
 *
 */

// General includes
#include <unistd.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/* POSIX Header files */
#include <pthread.h>

// FreeRTOS includes
#include <FreeRTOS.h>
#include <task.h>

void *sensorThread(void *arg0) {

    //sensorStruct myStruct;

    for(;;) {

        //if(receiveMsg(&myStruct))
            // Return error
        //BaseType_t
        continue;
    }

}

int createSensorThread(int threadStackSize, int prio) {

    pthread_t           thread;
    pthread_attr_t      attrs;
    struct sched_param  priParam;
    int                 retc;


    /* Initialize the attributes structure with default values */
    pthread_attr_init(&attrs);

    /* Set priority, detach state, and stack size attributes */
    priParam.sched_priority = prio;
    retc = pthread_attr_setschedparam(&attrs, &priParam);
    retc |= pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
    retc |= pthread_attr_setstacksize(&attrs, threadStackSize);
    if (retc != 0) {
        return -1; // Stack initialization failed
    }

    retc = pthread_create(&thread, &attrs, sensorThread, NULL);
    if (retc != 0) {
        return -2; // Thread/task creation failed
    }

    return 0;
}


int receiveMsg(void *rcv) {

    return 0;
}
