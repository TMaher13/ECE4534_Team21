#include <unistd.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
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
#include "lidar_task.h"

extern BaseType_t readQueue(QueueHandle_t handle, const void *data);
extern BaseType_t writeQueue(QueueHandle_t handle, const void *data);

extern QueueHandle_t lidar_handle;
extern QueueHandle_t nav_handle;
extern QueueHandle_t mqtt_handle;

extern void dbgEvent(unsigned int event);
extern void fatalError(unsigned int event);

typedef uint16_t fixed_point_t;

struct lidarData scanData[500];


void *lidarThread(void *arg0) {
    struct mqttQueueStruct publish;
    struct lidarQueueStruct lidarRequest; // = {TIMER70_MESSAGE, 0};
    //struct mqttQueueStruct mqttMessage;
    BaseType_t readRet, writeRet;
    uint16_t angle_q6, dist_q2;
    uint16_t angle_int, dist_int;
    char buffer[LIDAR_INFO_SIZE];
    unsigned char scan_buffer[LIDAR_SCAN_SIZE];
    char respDesc[7];
    const char healthMsg[] = "\xA5\x52";
    const char infoMsg[] = "\xA5\x50";
    const char scanMsg[] = "\xA5\x20";
    const char stopMsg[] = "\xA5\x25";

    int remaining;


    //GPIO_write(CONFIG_GPIO_MOTOCTL_8, CONFIG_GPIO_OUT_HIGH);

    UART_Handle uartHandle;
    UART_Params uartParams;
    UART_Params_init(&uartParams);
    //uartParams.writeMode = UART_MODE_BLOCKING;
    //uartParams.readMode = UART_MODE_BLOCKING;
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
    UART_write(uartHandle, stopMsg, LIDAR_REQUEST_SIZE);
    UART_PRINT("Opened UART2 for lidar task\r\n");

    writeRet = UART_write(uartHandle, healthMsg, LIDAR_REQUEST_SIZE);
    if(writeRet != LIDAR_REQUEST_SIZE) {
        //fatalError
    }
    UART_read(uartHandle, respDesc, LIDAR_RD_SIZE);
    if(respDesc[0] != (char)0xA5 ||
       respDesc[1] != (char)0x5A ||
       respDesc[2] != (char)0x3 ||
       respDesc[3] != (char)0x0 ||
       respDesc[4] != (char)0x0 ||
       respDesc[5] != (char)0x0 ||
       respDesc[6] != (char)0x6)
    {
        // fatalError
        UART_PRINT("Incorrect response descriptor on initial health check\r\n");
        return NULL;
    }

    UART_read(uartHandle, buffer, LIDAR_HEALTH_SIZE);
    if(buffer[0] != (char)0x0 ||
       buffer[1] != (char)0x0 ||
       buffer[2] != (char)0x0)
    {
        // fatalError
        UART_PRINT("Failed initial health check\r\n");
        return NULL;
    }
    UART_PRINT("Lidar passed initial health check\r\n");

    for(;;) {
        memset(respDesc, 0, LIDAR_REQUEST_SIZE);
        memset(buffer, 0, LIDAR_INFO_SIZE);
        memset(scan_buffer, 0, LIDAR_SCAN_SIZE);

        readRet = readQueue(lidar_handle, &lidarRequest);

        if(readRet == pdTRUE) {
            UART_PRINT("Received lidar request from MQTT\r\n");
            if(lidarRequest.messageType == MQTT_MESSAGE) {
                switch(lidarRequest.value) {

                case HEALTH_REQUEST:
                    writeRet = UART_write(uartHandle, healthMsg, LIDAR_REQUEST_SIZE);
                    if(writeRet != LIDAR_REQUEST_SIZE) {
                        //fatalError
                    }

                    UART_read(uartHandle, respDesc, LIDAR_RD_SIZE);
                    if(respDesc[0] == (char)0xA5) {
                    }
                    else {
                        UART_PRINT("Incorrect response descriptor for health request\r\n");
                    }

                    UART_read(uartHandle, buffer, LIDAR_HEALTH_SIZE);
                    if(buffer[0] != (char)0x0 ||
                       buffer[1] != (char)0x0 ||
                       buffer[2] != (char)0x0)
                    {
                        // fatalError
                        UART_PRINT("Failed health check\r\n");
                        return NULL;
                    }
                    UART_PRINT("Lidar sensor is healthy\r\n");

                    break;

                case INFO_REQUEST:
                    UART_PRINT("Sending ID info request to lidar\r\n");
                    if(UART_write(uartHandle, infoMsg, LIDAR_REQUEST_SIZE) != LIDAR_REQUEST_SIZE)
                        UART_PRINT("Failed to write 2 bytes to lidar\r\n");

                    UART_PRINT("Waiting for ID info from lidar\r\n");
                    UART_read(uartHandle, buffer, LIDAR_RD_SIZE);
                    UART_PRINT("Received response descriptor from LIDAR: %x\r\n", *buffer);
                    if(buffer[0] == (char)0xA5 && buffer[1] == (char)0x5A) {
                        memset(buffer, 0, LIDAR_INFO_SIZE);
                        UART_read(uartHandle, &buffer, LIDAR_INFO_SIZE);
                        UART_PRINT("ID data received from lidar: %s\r\n", buffer);
                    }
                    break;

                case SCAN_REQUEST:
                    UART_PRINT("Sending scan request to lidar\r\n");
                    writeRet = UART_write(uartHandle, scanMsg, LIDAR_REQUEST_SIZE);
                    if(writeRet != LIDAR_REQUEST_SIZE) {
                        //fatalError
                    }

                    UART_read(uartHandle, respDesc, LIDAR_RD_SIZE);
                    if(respDesc[0] != (char)0xA5) {
                        UART_PRINT("Wrong response descriptor\r\n");
                        return NULL;
                    }
                    /*else if(respDesc[0] == (char)0x5A) {
                        // Do nothing for now
                    }
                    else {
                        // fatalError
                        UART_PRINT("Not the correct response descriptor: %x\r\n", *respDesc);
                        return NULL;
                    }*/
                    UART_PRINT("Beginning scan\r\n");
                    uint32_t i;
                    for(i=0; i<500; ++i) {
                        UART_read(uartHandle, scan_buffer, LIDAR_SCAN_SIZE);

                        //UART_PRINT("Quality & start: %x\r\n", *scan_buffer);
                        //scanData[i] = (uint_least8_t)scan_buffer[0];

                        scanData[i].angle = (scan_buffer[2]<<7 |  scan_buffer[1]>>1) / 64; // Remove check bit
                        //angle_int = (angle_q6 / (1 << FP_ANGLE_FRACTION_BITS));

                        scanData[i].dist = (scan_buffer[4] << 8 | scan_buffer[3]) / 4;

                        //dist_int = (dist_q2 / (1 << FP_DIST_FRACTION_BITS));
                        //if(angle_q6 > 360 || angle_q6 < 0)
                        //    UART_PRINT("[%d] You're still doing it wrong :/\r\n", i);
                        //else
                        //    distData[angle_int] = dist_int;
                        //UART_PRINT("[%d] Angle %u (degrees): %umm\r\n", i, angle_q6, dist_q2);
                    }

                    if(UART_write(uartHandle, stopMsg, LIDAR_REQUEST_SIZE) != LIDAR_REQUEST_SIZE)
                        UART_PRINT("Failed to write 2 bytes to lidar\r\n");

                    UART_control(uartHandle, UART_CMD_GETRXCOUNT, &remaining);
                    while(remaining != 0) {
                        UART_PRINT("Flushing %d bytes\r\n", remaining);
                        UART_read(uartHandle, NULL, remaining);
                        UART_control(uartHandle, UART_CMD_GETRXCOUNT, &remaining);
                    }
                    UART_PRINT("Finished scanning\r\n");

                    snprintf(publish.topic, TOPIC_SIZE, "scan_results");

                    for(i=0; i<500; ++i) {
                        snprintf(publish.payload, PAYLOAD_SIZE, "%d,%d", scanData[i].angle, scanData[i].dist);

                        //send to publish queue
                        writeQueue(mqtt_handle, &publish);
                    }

                    break;
                case STOP_REQUEST:
                    UART_PRINT("Sending stop request to lidar\r\n");
                    if(UART_write(uartHandle, stopMsg, LIDAR_REQUEST_SIZE) != LIDAR_REQUEST_SIZE)
                        UART_PRINT("Failed to write 2 bytes to lidar\r\n");

                    UART_control(uartHandle, UART_CMD_GETRXCOUNT, &remaining);
                    while(remaining != 0) {
                        UART_PRINT("Flushing %d bytes\r\n", remaining);
                        UART_read(uartHandle, NULL, remaining);
                        UART_control(uartHandle, UART_CMD_GETRXCOUNT, &remaining);
                    }

                    break;
                default:
                    //UART_PRINT("Unknown request received\r\n");
                    break;
                }

            }
            else {
                UART_PRINT("Received unknown message from mqtt task\r\n");
            }

        }
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

