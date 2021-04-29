/*
 * Arm_MQTT.c
 *
 *  Created on: Mar 19, 2021
 *      Author: Connor Bondi
 */

#include <unistd.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

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
#include "uart_term.h"
//#include <sensor_thread_queue.h>
//#include <sensor_thread_state.h>

#define PAYLOAD_SIZE 256

extern BaseType_t readQueue(QueueHandle_t handle, void * const data);
extern BaseType_t writeQueue(QueueHandle_t handle, void * const data);

extern QueueHandle_t receive_handle;
extern QueueHandle_t publish_handle;

void sendArmStateMessage(char* msg){

    BaseType_t publishQueueRet;
    static struct publishQueueStruct publish;
    char jsonStr[PAYLOAD_SIZE];

    //set topic
    snprintf(publish.topic, TOPIC_SIZE, "Arm_State");

    //set payload
    memset(jsonStr, 0, PAYLOAD_SIZE);
    snprintf(jsonStr, PAYLOAD_SIZE, "{\"ArmState\":\"%s\"}", msg);
    memcpy(publish.payload,jsonStr, PAYLOAD_SIZE);

    //send to publish queue
    publishQueueRet = writeQueue(publish_handle, &publish);
    if (publishQueueRet == 0){
        UART_PRINT("\n\rNOT SENT");
    }

}
