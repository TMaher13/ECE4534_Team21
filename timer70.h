#include <stddef.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/Timer.h>
#include <ti/drivers/ADC.h>

#include "ti_drivers_config.h"

#include "queue_structs.h"
#include "sensor_thread_queue.h"

#ifndef TIMER70
#define TIMER70

extern const uint32_t          TIMER70_PERIOD_CONST;
#define TIMER70_PERIOD         70000

/*!
 *  @brief  Function to perform a conversion into millimeters
 *
 *  @param[in]      adcValue  A value to be converted
 *
 *  @retval         adcValue converted into millimeters
*/
uint32_t convert2mm(uint16_t adcValue);

void timer70Init();

/*
 * This callback is called every 1,000,000 microseconds, or 1 second. Because
 * the LED is toggled each time this function is called, the LED will blink at
 * a rate of once every 2 seconds.
 */
void timer70Callback(Timer_Handle myHandle, int_fast16_t status);

#endif
