#include <stdint.h>
#include <stddef.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/Timer.h>
#include <ti/drivers/ADC.h>

/* RTOS header files */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include <queue_structs.h>
#include <debug.h>

#include <ti_drivers_config.h>

extern const uint32_t          TIMER70_PERIOD_CONST;
#define TIMER70_PERIOD         70000

extern void dbgEvent(unsigned int event);
extern BaseType_t writeSensorQueueCallback(const void *pvItemToQueue);

/*
 * This callback is called every 1,000,000 microseconds, or 1 second. Because
 * the LED is toggled each time this function is called, the LED will blink at
 * a rate of once every 2 seconds.
 */
void timer70Callback(Timer_Handle myHandle, int_fast16_t status)
{

    dbgEvent(ENTER_TIMER70_CALLBACK);

    int_fast16_t res;
    uint16_t adcValue;
    uint32_t mmValue = 00;

    static uint32_t spoofReading = 0;

/*    ADC_Params params;
    ADC_Handle adc;

    ADC_Params_init(&params);
    adc = ADC_open(CONFIG_ADC_0, &params);

     (adc == NULL) {
         Failed to initialized timer
        while (1);
    }

    res = ADC_convert(0, &adcValue);

    if (res == ADC_STATUS_SUCCESS)
    {
        mmValue = convert2mm(adcValue);

    }
    else {
        mmValue = spoofReading;
    }

    spoofReading++;*/
    struct sensorQueueStruct m = {TIMER70_MESSAGE, mmValue};

    writeSensorQueueCallback(&m);

    //ADC_close(adc);

    dbgEvent(LEAVE_TIMER70_CALLBACK);
}

void timer70Init()
{
    Timer_Handle timer70;
    Timer_Params params;

    /* Call driver init functions */
    Timer_init();

    /*
     * Setting up the timer in continuous callback mode that calls the callback
     * function every 1,000,000 microseconds, or 1 second.
     */
    Timer_Params_init(&params);
    params.period = TIMER70_PERIOD;
    params.periodUnits = Timer_PERIOD_US;
    params.timerMode = Timer_CONTINUOUS_CALLBACK;
    params.timerCallback = timer70Callback;

    timer70 = Timer_open(CONFIG_TIMER_2, &params);

    if (timer70 == NULL) {
        /* Failed to initialized timer */
        while (1) {}
    }

    if (Timer_start(timer70) == Timer_STATUS_ERROR) {
        /* Failed to start timer */
        while (1) {}
    }
}

uint32_t convert2mm(uint16_t adcValue)
{
    return adcValue * 10;
}



