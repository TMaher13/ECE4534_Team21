#include <unistd.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
/* POSIX Header files */
#include <pthread.h>

/* RTOS header files */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include <ti/drivers/GPIO.h>
#include <ti_drivers_config.h>

#include <queue_structs.h>
#include <debug.h>

extern BaseType_t readQueue(QueueHandle_t handle, const void *data);
extern BaseType_t writeQueue(QueueHandle_t handle, const void *data);

extern QueueHandle_t nav_handle;
extern QueueHandle_t mqtt_handle;

extern void dbgEvent(unsigned int event);
extern void fatalError(unsigned int event);


void *navigationThread(void *arg0) {
    static struct navQueueStruct navData; // = {TIMER70_MESSAGE, 0};
    BaseType_t readRet;

    for(;;) {

        readRet = readQueue(nav_handle, &navData);

        if(readRet == pdTRUE) {


        }
    }
}


int createNavigationThread(int threadStackSize, int prio) {

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
        //fatalError(SENSOR_STACK_FATAL_ERROR);
        return -1; // Stack initialization failed
    }

    retc = pthread_create(&thread, &attrs, navigationThread, NULL);
    if (retc != 0) {
        //fatalError(SENSOR_THREAD_FATAL_ERROR);
        return -2; // Thread/task creation failed
    }

    return 0;
}
