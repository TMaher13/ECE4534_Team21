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

extern const uint32_t          TIMER500_PERIOD_CONST;
#define TIMER500_PERIOD        500000

extern void timer500Callback(Timer_Handle myHandle, int_fast16_t status);
extern BaseType_t writeSensorQueueCallback(const void *pvItemToQueue);

extern void dbgEvent(unsigned int event);
extern void fatalError(unsigned int event);

void timer500Init()
{
    Timer_Handle timer500;
    Timer_Params params;

    /* Call driver init functions */
    Timer_init();

    Timer_Params_init(&params);
    params.period = TIMER500_PERIOD;
    params.periodUnits = Timer_PERIOD_US;
    params.timerMode = Timer_CONTINUOUS_CALLBACK;
    params.timerCallback = timer500Callback;

    timer500 = Timer_open(CONFIG_TIMER_1, &params);

    if (timer500 == NULL) {
        /* Failed to initialized timer */
        fatalError(TIMER500_INIT_FATAL_ERROR);
        while (1) {}
    }

    if (Timer_start(timer500) == Timer_STATUS_ERROR) {
        /* Failed to start timer */
        fatalError(TIMER500_START_FATAL_ERROR);
        while (1) {}
    }

}

void timer500Callback(Timer_Handle myHandle, int_fast16_t status)
{
    dbgEvent(ENTER_TIMER500_CALLBACK);

    static struct sensorQueueStruct m;

    m.messageType = TIMER500_MESSAGE;

    writeSensorQueueCallback(&m);

    dbgEvent(LEAVE_TIMER500_CALLBACK);
}
