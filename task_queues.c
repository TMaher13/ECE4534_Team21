/*
 * Queue create/write/read for tasks to communicate with each other and callbacks
 *
 * Author: Thomas Maher
 * Date modified: 3/3/2021
 *
 */

/* RTOS header files */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>


QueueHandle_t createQueue(unsigned int queueLen, unsigned int itemSize) {
    return xQueueCreate(queueLen, itemSize);
}


BaseType_t readQueue(QueueHandle_t handle, const void *data) {

    return xQueueReceive(handle, data, 100);
}

BaseType_t writeQueue(QueueHandle_t handle, const void *data) {

    return xQueueSend(handle, data, 100);
}

BaseType_t writeQueueCallback(QueueHandle_t handle, const void *m) {
    BaseType_t res, xHigherPriorityTaskWoken;
    return xQueueSendFromISR(handle, m, &xHigherPriorityTaskWoken);

}

