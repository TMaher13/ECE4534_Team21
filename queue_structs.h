#include <stdlib.h>
#include <stdint.h>

extern const uint_least8_t          TIMER70_MESSAGE_CONST;
#define TIMER70_MESSAGE             0

extern const uint_least8_t          TIMER500_MESSAGE_CONST;
#define TIMER500_MESSAGE            1

struct sensorQueueStruct {

    uint_least8_t messageType;
    uint32_t value;

};

struct uartQueueStruct {

    char* msg;

};
