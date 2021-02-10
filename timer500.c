#include "timer500.h"

void timer500Init()
{
    Timer_Handle timer500;
    Timer_Params params;

    /* Call driver init functions */
    Timer_init();

    /*
     * Setting up the timer in continuous callback mode that calls the callback
     * function every 1,000,000 microseconds, or 1 second.
     */
    Timer_Params_init(&params);
    params.period = 500000;
    params.periodUnits = Timer_PERIOD_US;
    params.timerMode = Timer_CONTINUOUS_CALLBACK;
    params.timerCallback = timer500Callback;

    timer500 = Timer_open(CONFIG_TIMER_0, &params);

    if (timer500 == NULL) {
        /* Failed to initialized timer */
        while (1) {}
    }

    if (Timer_start(timer500) == Timer_STATUS_ERROR) {
        /* Failed to start timer */
        while (1) {}
    }

}

uint32_t convertTicks2ms(TickType_t ticks)
{
    return ticks * 1000;
}

/*
 * This callback is called every 1,000,000 microseconds, or 1 second. Because
 * the LED is toggled each time this function is called, the LED will blink at
 * a rate of once every 2 seconds.
 */
void timer500Callback(Timer_Handle myHandle, int_fast16_t status)
{
    TickType_t tickCount = xTaskGetTickCountFromISR();

    uint32_t msec = convertTicks2ms(tickCount);

    uint32_t elapsed; //= elapsedTime(msec);

    struct sensorQueueStruct m = {TIMER500_MESSAGE, elapsed};
    writeSensorQueueCallback(&m);
}

