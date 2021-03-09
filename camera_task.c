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

#include "uart_term.h"

extern BaseType_t readQueue(QueueHandle_t handle, const void *data);
extern BaseType_t writeQueue(QueueHandle_t handle, const void *data);

extern QueueHandle_t nav_handle;
extern QueueHandle_t mqtt_handle;

extern void dbgEvent(unsigned int event);
extern void fatalError(unsigned int event);


//static void i2cErrorHandler(I2C_Transaction *transaction);


void *cameraThread(void *arg0) {
    struct navQueueStruct cameraNavData;
    BaseType_t writeRet;

    I2C_Handle      i2c;
    I2C_Params      i2cParams;
    I2C_Transaction i2cTransaction;
    uint8_t         txBuffer;
    uint8_t         rxBuffer;
    volatile int    test = 0;

    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
    i2c = I2C_open(CONFIG_I2C_0, &i2cParams);
    if (i2c == NULL) {
        UART_PRINT("Error Initializing I2C\r\n");
        while (1);
    }
    else {
        UART_PRINT("I2C Initialized!\r\n");
    }

    for(;;) {
        test = 4;
    }

    // Common I2C transaction setup
    //i2cTransaction.writeBuf   = txBuffer;
    //i2cTransaction.writeCount = 1;
    //i2cTransaction.readBuf    = rxBuffer;
    //i2cTransaction.readCount  = 0;

    /*i2cTransaction.slaveAddress = I2C_SLAVE_ADDRESS;

    if (I2C_transfer(i2c, &i2cTransaction)) {
        UART_PRINT("Detected LIDAR sensor with slave"
                   " address 0x%x", I2C_SLAVE_ADDRESS);
    }
    else {
        i2cErrorHandler(&i2cTransaction);
    }

    i2cTransaction.writeCount = 0;
    i2cTransaction.readCount = CAMERA_PACKET_SIZE;

    for(;;) {

        if (I2C_transfer(i2c, &i2cTransaction)) {


            writeRet = writeQueue(nav_handle, &cameraNavData);
        }
        else {
            i2cErrorHandler(&i2cTransaction);
        }
    }*/

    return NULL;
}


int createCameraThread(int threadStackSize, int prio) {
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

    retc = pthread_create(&thread, &attrs, cameraThread, NULL);
    if (retc != 0) {
        //fatalError(SENSOR_THREAD_FATAL_ERROR);
        return -2; // Thread/task creation failed
    }

    return 0;
}



/*static void i2cErrorHandler(I2C_Transaction *transaction) {
    switch (transaction->status) {
        case I2C_STATUS_TIMEOUT:
            UART_PRINT("I2C transaction timed out!\r\n");
            break;
        case I2C_STATUS_CLOCK_TIMEOUT:
            UART_PRINT("I2C serial clock line timed out!\r\n");
            break;
        case I2C_STATUS_ADDR_NACK:
            UART_PRINT("I2C slave address 0x%x not acknowledged!\r\n", transaction->slaveAddress);
            break;
        case I2C_STATUS_DATA_NACK:
            UART_PRINT("I2C data byte not acknowledged!\r\n");
            break;
        case I2C_STATUS_ARB_LOST:
            UART_PRINT("I2C arbitration to another master!\r\n");
            break;
        case I2C_STATUS_INCOMPLETE:
            UART_PRINT("I2C transaction returned before completion!\r\n");
            break;
        case I2C_STATUS_BUS_BUSY:
            UART_PRINT("I2C bus is already in use!\r\n");
            break;
        case I2C_STATUS_CANCEL:
            UART_PRINT("I2C transaction cancelled!\r\n");
            break;
        case I2C_STATUS_INVALID_TRANS:
            UART_PRINT("I2C transaction invalid!\r\n");
            break;
        case I2C_STATUS_ERROR:
            UART_PRINT("I2C generic error!\r\n");
            break;
        default:
            UART_PRINT("I2C undefined error case!\r\n");
            break;
    }
}*/
