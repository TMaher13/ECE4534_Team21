#include <stddef.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/Timer.h>

#include "ti_drivers_config.h"

#ifndef TIMER500
#define TIMER500

void timer1000Init();

void timer1000Callback(Timer_Handle myHandle, int_fast16_t status);

#include <timer1000.c>

#endif
