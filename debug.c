/*
 *  ======== debug.c ========
 */

/* General Includes */
#include <unistd.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "debug.h"

/* POSIX Header files */
#include <pthread.h>

/* FreeRTOS includes */
#include <FreeRTOS.h>
#include <task.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/Timer.h>

/* Board Header file */
#include "ti_drivers_config.h"

/* Callback used for toggling the LED. */
void timerCallback(Timer_Handle myHandle, int_fast16_t status);

void dbgEvent(unsigned int event){
    //check for event <= 127
    //call fail routine if not
    if (event > EVENT_VALUE_LIMIT){
        fatalError(EVENT_VALUE_ERROR);
    }

    //output to set of 8 GPIO lines
    dbgGPIOWrite(event);

}

void fatalError(unsigned int event){
    //must make sure everything in the code stops
    taskENTER_CRITICAL();
    vTaskSuspendAll();

    //error event must be written to GPIO lines using dbgEvent routine
    dbgEvent(event);

    //must blink an LED
    while(1){
        //errorLED();
        GPIO_toggle(CONFIG_GPIO_LED_0);
        vTaskDelay(1000);
    }

    //error code should be last thing written to GPIO lines
}

void debugInit(){
    GPIO_init();

    /* Configure the LED pin */
    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    /* Configure the GPIO pins */
    GPIO_setConfig(CONFIG_GPIO_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_1, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_2, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_3, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_4, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_5, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_6, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_7, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    /* Turn off user LED */
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF);
}

void dbgGPIOWrite(unsigned int event){

    //Set 8th bit high while writing
    GPIO_write(CONFIG_GPIO_7, CONFIG_GPIO_OUT_HIGH);

    /* Fill array called bits with the binary value of event, 0 is LSB, 6 is MSB */
    int bits[7];
    int i;
    for(i = 0; i < 7; i++){
        bits[i] = (event >> i) & 1;
    }

    /* Turn on the pins representing event's value MSB = 6, LSB = 0 */
    GPIO_write(CONFIG_GPIO_0, bits[0]);
    GPIO_write(CONFIG_GPIO_1, bits[1]);
    GPIO_write(CONFIG_GPIO_2, bits[2]);
    GPIO_write(CONFIG_GPIO_3, bits[3]);
    GPIO_write(CONFIG_GPIO_4, bits[4]);
    GPIO_write(CONFIG_GPIO_5, bits[5]);
    GPIO_write(CONFIG_GPIO_6, bits[6]);

    //Set 8th bit low after writing
    GPIO_write(CONFIG_GPIO_7, CONFIG_GPIO_OUT_LOW);
}
