#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/Timer.h>
#include <ti/drivers/ADC.h>

/* RTOS header files */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <debug.h>
#include <queue_structs.h>

#include <ti_drivers_config.h>

extern const uint32_t          TIMER70_PERIOD_CONST;
#define TIMER70_PERIOD         70000

extern void timer70Callback(Timer_Handle myHandle, int_fast16_t status);
extern BaseType_t writeSensorQueueCallback(const void *pvItemToQueue);
extern void dbgEvent(unsigned int event);
extern void fatalError(unsigned int event);

void timer70Init()
{
    Timer_Handle timer70;
    Timer_Params params;

    /* Call driver init functions */
    Timer_init();

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

    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    if (Timer_start(timer70) == Timer_STATUS_ERROR) {
        /* Failed to start timer */
        while (1) {
            //GPIO_toggle(CONFIG_GPIO_LED_0);
            //sleep(1);
        }
    }
}

void timer70Callback(Timer_Handle myHandle, int_fast16_t status)
{
    dbgEvent(ENTER_TIMER70_CALLBACK);

    static struct sensorQueueStruct m;

    m.messageType = TIMER70_MESSAGE;

    writeSensorQueueCallback(&m);

    dbgEvent(LEAVE_TIMER70_CALLBACK);
}
