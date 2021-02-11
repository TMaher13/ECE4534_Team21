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

void dbgEvent(unsigned int event);
void fatalError(unsigned int event);
void dbgGPIOWrite(unsigned int event);
void errorLED();



#endif
