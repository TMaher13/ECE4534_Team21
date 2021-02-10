#include "timer70.h"

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

    timer70 = Timer_open(CONFIG_TIMER_0, &params);

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

/*
 * This callback is called every 1,000,000 microseconds, or 1 second. Because
 * the LED is toggled each time this function is called, the LED will blink at
 * a rate of once every 2 seconds.
 */
void timer70Callback(Timer_Handle myHandle, int_fast16_t status)
{
    int_fast16_t res;
    uint16_t adcValue;
    uint32_t mmValue;

    ADC_Params params;
    ADC_Handle adc;

    ADC_Params_init(&params);
    adc = ADC_open(CONFIG_ADC_0, &params);

    if (adc == NULL) {
        /* Failed to initialized timer */
        while (1);
    }

    res = ADC_convert(0, &adcValue);
    mmValue = convert2mm(adcValue);

    if (res == ADC_STATUS_SUCCESS)
    {
        struct sensorQueueStruct m = {TIMER70_MESSAGE, mmValue};

        writeSensorQueueCallback(&m);
    }
    else {
        /* Failed to initialized timer */
        while (1);
    }

    ADC_close(adc);
}

