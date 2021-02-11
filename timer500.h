#include <stddef.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/Timer.h>
#include <ti/drivers/ADC.h>

#include "ti_drivers_config.h"

#ifndef TIMER500
#define TIMER500

void timer500Init();

uint32_t convertTicks2ms(TickType_t ticks);


/*
 * This callback is called every 1,000,000 microseconds, or 1 second. Because
 * the LED is toggled each time this function is called, the LED will blink at
 * a rate of once every 2 seconds.
 */
void timer500Callback(Timer_Handle myHandle, int_fast16_t status);

#include <timer500.c>


#endif
