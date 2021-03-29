#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <ti/drivers/ADC.h>
#include <queue_structs.h>
#include <debug.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

/* Driver configuration */
#include "ti_drivers_config.h"

extern writeQueue(QueueHandle_t handle, void * const data);

extern void dbgEvent(unsigned int event);
extern void fatalError(unsigned int event);


/* ADC sample count */
#define ADC_SAMPLE_COUNT  (10)

void sensorFSM(QueueHandle_t publish_handle, struct sensorQueueStruct *sensorMsg) {

    static int numMessages = 0;
    static int total = 0;
    static int avg = 0;
    static int adcValue = 0;

    //create payload (JSON String)
    static struct publishQueueStruct publish;
    static char jsonStr[PAYLOAD_SIZE];

    static int messageID = 0;

    switch(sensorMsg->messageType) {

        case TIMER70_MESSAGE:
        {
        ADC_Handle adc;
        ADC_Params params;
        int_fast16_t res;

        ADC_Params_init(&params);
        adc = ADC_open(CONFIG_ADC_0, &params);

        if (adc == NULL)
        {
            while (1);
        }

        res = ADC_convert(adc, &adcValue);

        ADC_close(adc);

        /*
         * CONVERT ADC TO MM
         *
         *
         */

        total += adcValue;
        numMessages++;


        //set topic
        snprintf(publish.topic, TOPIC_SIZE, "joseph_sensor");

        //set payload
        memset(jsonStr, 0, PAYLOAD_SIZE);
        snprintf(
                jsonStr,
                PAYLOAD_SIZE,
                "{\"messageType\":\"%d\",\"messageID\":\"%d\",\"value1\":\"%d\",\"value2\":\"%d\"}",
                TIMER70_MESSAGE,
                messageID, adcValue, 0);
        memcpy(publish.payload, jsonStr, PAYLOAD_SIZE);

        //write to publish queue
        writeQueue(publish_handle, &publish);

        break;
        }
        case TIMER500_MESSAGE:
        {
            avg = total/numMessages;

            //set topic
            snprintf(publish.topic, TOPIC_SIZE, "joseph_sensor");

            //set payload
            memset(jsonStr, 0, PAYLOAD_SIZE);
            snprintf(
                    jsonStr,
                    PAYLOAD_SIZE,
                    "{\"messageType\":\"%d\",\"messageID\":\"%d\",\"value1\":\"%d\",\"value2\":\"%d\"}",
                    TIMER500_MESSAGE, messageID, avg, 1);
            memcpy(publish.payload, jsonStr, PAYLOAD_SIZE);

            //write to publish queue
            writeQueue(publish_handle, &publish);

            numMessages = 0;
            total = 0;
            avg = 0;

            break;

        }
        case IPS_MESSAGE:
                {
                ADC_Handle adc;
                ADC_Params params;
                int_fast16_t res;

                static int isMetal = 0;

                ADC_Params_init(&params);
                adc = ADC_open(CONFIG_ADC_1, &params);

                if (adc == NULL)
                {
                    while (1);
                }

                res = ADC_convert(adc, &adcValue);

                ADC_close(adc);

                if (adcValue > 3000){
                    isMetal = 0;
                }
                else if (adcValue < 100){
                    isMetal = 1;
                }
                else {
                    isMetal = -1;
                }

                //set topic
                snprintf(publish.topic, TOPIC_SIZE, "joseph_sensor");

                //set payload
                memset(jsonStr, 0, PAYLOAD_SIZE);
                snprintf(
                        jsonStr,
                        PAYLOAD_SIZE,
                        "{\"messageType\":\"%d\",\"messageID\":\"%d\",\"isMetal\":\"%d\"}",
                        IPS_MESSAGE,
                        messageID, isMetal);
                memcpy(publish.payload, jsonStr, PAYLOAD_SIZE);

                //write to publish queue
                writeQueue(publish_handle, &publish);

                break;
          }
    }
    messageID++;
}
