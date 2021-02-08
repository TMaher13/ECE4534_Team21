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

// TI includes
#include <ti/drivers/ADC.h>
#include "ti_drivers_config.h"

// FreeRTOS includes
#include "FreeRTOS.h"
#include "task.h"

// Sensor includes
#include <sensor_thread_queue.h>
#include <sensor_thread_state.h>


void *sensorThread(void *arg0) {

    sensorStruct myStruct;

    for(;;) {

        if(receiveMsg(&myStruct))
            // Return error

    }

}


int receiveMsg(sensorStruct *rcv) {

    xQueueReceive( xQueue, &ulReceivedValue, portMAX_DELAY );
}
