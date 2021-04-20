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

#include <uart_term.h>
#include <queue_structs.h>
#include <task_queues.h>
#include <debug.h>
#include <ti_drivers_config.h>

extern const uint32_t          TIMER1000_PERIOD_CONST;
#define TIMER1000_PERIOD        1000000

extern QueueHandle_t lidar_handle;

extern void timer1000Callback(Timer_Handle myHandle, int_fast16_t status);
extern BaseType_t writeQueueCallback(QueueHandle_t handle, const void *m);

extern void dbgEvent(unsigned int event);
extern void fatalError(unsigned int event);

void timer1000Init()
{
    Timer_Handle timer1000;
    Timer_Params params;

    /* Call driver init functions */
    Timer_init();

    Timer_Params_init(&params);
    params.period = TIMER1000_PERIOD;
    params.periodUnits = Timer_PERIOD_US;
    params.timerMode = Timer_CONTINUOUS_CALLBACK;
    params.timerCallback = timer1000Callback;

    timer1000 = Timer_open(CONFIG_TIMER_1, &params);

    if (timer1000 == NULL) {
        /* Failed to initialized timer */
        //fatalError(TIMER1000_INIT_FATAL_ERROR);
        while (1) {}
    }

    if (Timer_start(timer1000) == Timer_STATUS_ERROR) {
        /* Failed to start timer */
        //fatalError(TIMER1000_START_FATAL_ERROR);
        while (1) {}
    }

}


void timer1000Callback(Timer_Handle myHandle, int_fast16_t status)
{
    //dbgEvent(ENTER_TIMER1000_CALLBACK);
    BaseType_t writeRet;

    static struct lidarQueueStruct lidarRequest;

    lidarRequest.messageType = TIMER_MESSAGE;
    lidarRequest.value = SCAN_REQUEST;

    writeRet = writeQueueCallback(lidar_handle, &lidarRequest);
    if(writeRet == errQUEUE_FULL) {
        UART_PRINT("Lidar queue full\r\n");
    }
}
