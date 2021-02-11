/*
 *  ======== debug.h ========
 */

#ifndef DEBUG_H
#define DEBUG_H

#define CONFIG_GPIO_OUT_HIGH (1)
#define CONFIG_GPIO_OUT_LOW (0)

/* Debugging Constants */
#define TEST_DBG 0xA //10
#define EVENT_VALUE_LIMIT 0x7F //127
#define EVENT_VALUE_ERROR 0x63 //99

#define ENTER_SENSOR_TASK 0x1
#define BEFORE_SENSOR_LOOP 0x2
#define BEFORE_READ_SENSOR_QUEUE 0x3
#define AFTER_READ_SENSOR_QUEUE 0x4
#define BEFORE_WRITE_UART_QUEUE_TIMER500 0x5
#define AFTER_WRITE_UART_QUEUE_TIMER500 0x6
#define BEFORE_WRITE_UART_QUEUE_TIMER70 0x7
#define AFTER_WRITE_UART_QUEUE_TIMER70 0x8
#define BEFORE_WRITE_UART_QUEUE 0x9
#define AFTER_WRITE_UART_QUEUE 0xA
#define ENTER_UART_TASK 0xB
#define BEFORE_UART_LOOP 0xC
#define BEFORE_READ_UART_QUEUE 0xD
#define AFTER_READ_UART_QUEUE 0xE
#define ENTER_TIMER500_CALLBACK 0xF
#define LEAVE_TIMER500_CALLBACK 0x10
#define ENTER_TIMER70_CALLBACK 0x11
#define LEAVE_TIMER70_CALLBACK 0x12
#define ENTER_SENSOR_QUEUE_CALLBACK 0x13
#define LEAVE_SENSOR_QUEUE_CALLBACK 0x13


void dbgEvent(unsigned int event);
void fatalError(unsigned int event);
void dbgGPIOWrite(unsigned int event);
void errorLED();
void debugInit();



#endif
