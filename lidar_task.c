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
#include <ti/drivers/UART.h>
#include <ti_drivers_config.h>

#include <queue_structs.h>
#include <debug.h>

#include "uart_term.h"

extern BaseType_t readQueue(QueueHandle_t handle, const void *data);
extern BaseType_t writeQueue(QueueHandle_t handle, const void *data);

extern QueueHandle_t lidar_handle;
extern QueueHandle_t nav_handle;
extern QueueHandle_t mqtt_handle;

extern void dbgEvent(unsigned int event);
extern void fatalError(unsigned int event);


void *lidarThread(void *arg0) {
    struct lidarQueueStruct lidarRequest; // = {TIMER70_MESSAGE, 0};
    //struct mqttQueueStruct mqttMessage;
    BaseType_t readRet, writeRet;
    char buffer[LIDAR_INFO_SIZE];
    const char healthMsg[] = "\xA5\x52"; // Because of little endianness
    const char infoMsg[] = "\xA5\x50";
    const char scanMsg[2] = "\xA5\x20";

    //GPIO_write(CONFIG_GPIO_MOTOCTL_8, CONFIG_GPIO_OUT_HIGH);

    UART_Handle uartHandle;
    UART_Params uartParams;
    UART_Params_init(&uartParams);
    uartParams.writeMode = UART_MODE_BLOCKING;
    uartParams.readMode = UART_MODE_BLOCKING;
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.baudRate = 115200;
    uartParams.stopBits = UART_STOP_ONE;
    uartParams.parityType = UART_PAR_NONE;
    uartParams.dataLength = UART_LEN_8;
    uartHandle = UART_open(CONFIG_UART_0, &uartParams);
    if(!uartHandle) {
        //fatalError(UART_INIT_FATAL_ERROR);
        UART_PRINT("Failed to open UART2 from lidar task\r\n");
    }
    UART_PRINT("Opened UART2 for lidar task\r\n");

    for(;;) {
        readRet = readQueue(lidar_handle, &lidarRequest);

        if(readRet == pdTRUE) {
            UART_PRINT("Received lidar request from MQTT\r\n");
            if(lidarRequest.messageType == MQTT_MESSAGE) {
                switch(lidarRequest.value) {

                case HEALTH_REQUEST:
                    UART_PRINT("Sending health request to lidar\r\n");
                    writeRet = UART_write(uartHandle, "\xA5\x52", 2);
                    if(writeRet != 2) {
                        //fatalError
                    }

                    UART_PRINT("Waiting for health info from lidar (sent %d bytes)\r\n", writeRet);
                    UART_read(uartHandle, &buffer, LIDAR_RD_SIZE);
                    UART_PRINT("Received response descriptor from LIDAR\r\n");
                    if(buffer[0] == (char)0xA5 && buffer[1] == (char)0x5A) {
                        memset(buffer, 0, LIDAR_PACKET_SIZE);
                        UART_read(uartHandle, &buffer, LIDAR_HEALTH_SIZE);
                        UART_PRINT("Health data received from lidar: %x\r\n", buffer);
                    }
                    break;

                case INFO_REQUEST:
                    UART_PRINT("Sending ID info request to lidar\r\n");
                    if(UART_write(uartHandle, infoMsg, LIDAR_REQUEST_SIZE) != LIDAR_REQUEST_SIZE)
                        UART_PRINT("Failed to write 2 bytes to lidar\r\n");

                    UART_PRINT("Waiting for ID info from lidar\r\n");
                    UART_read(uartHandle, &buffer, LIDAR_RD_SIZE);
                    UART_PRINT("Received response descriptor from LIDAR\r\n");
                    if(buffer[0] == (char)0xA5 && buffer[0] == (char)0x5A) {
                        memset(buffer, 0, LIDAR_INFO_SIZE);
                        UART_read(uartHandle, &buffer, LIDAR_INFO_SIZE);
                        UART_PRINT("ID data received from lidar: %x\r\n", buffer);
                    }
                    break;

                case SCAN_REQUEST:

                    break;
                case STOP_REQUEST:

                    break;
                default:
                    //UART_PRINT("Unknown request received\r\n");
                    break;
                }

            }
            else {
                //UART_PRINT("Received unknown message from mqtt task\r\n");
            }

        }

        /*UART_PRINT("Waiting to read from LIDAR\r\n");
        memset(buffer, 0, LIDAR_PACKET_SIZE);
        UART_read(uartHandle, &buffer, LIDAR_RD_SIZE);

        if(buffer[0] == 0xA55A) {
            UART_PRINT("Response Descriptor Received\r\n");

            UART_read(uartHandle, &buffer, LIDAR_HEALTH_SIZE);

            UART_PRINT("Health data received from lidar: %x\r\n", buffer);

        }
        else {
            UART_PRINT("Unknown dtaa received: %d\r\n", buffer[0]);
        }*/


        //writeRet = writeQueue(nav_handle, &lidarNavData);
        //if(writeRet == errQUEUE_FULL) {
            //fatalError();
        //}
    }
}


int createLidarThread(int threadStackSize, int prio) {

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

    retc = pthread_create(&thread, &attrs, lidarThread, NULL);
    if (retc != 0) {
        //fatalError(SENSOR_THREAD_FATAL_ERROR);
        return -2; // Thread/task creation failed
    }

    return 0;
}

