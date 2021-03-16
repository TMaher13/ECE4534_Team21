#ifndef QUEUE_STRUCTS
#define QUEUE_STRUCTS

#include <stdlib.h>
#include <stdint.h>

extern const uint_least8_t          TIMER70_MESSAGE_CONST;
#define TIMER70_MESSAGE             0

extern const uint_least8_t          TIMER500_MESSAGE_CONST;
#define TIMER500_MESSAGE            1

extern const uint_least8_t          TIMER1000_MESSAGE_CONST;
#define TIMER1000_MESSAGE           2

extern const uint_least8_t          TOPIC_SIZE_CONST;
#define TOPIC_SIZE                  64

extern const uint_least8_t          PAYLOAD_SIZE_CONST;
#define PAYLOAD_SIZE                256

extern const uint_least8_t          SECRET_SIZE_CONST;
#define SECRET_SIZE                 256

struct sensorQueueStruct {

    uint_least8_t messageType;
    uint32_t value;

};

struct publishQueueStruct {

    char topic[TOPIC_SIZE];
    char payload[PAYLOAD_SIZE];

};

struct chainQueueStruct {
    char secret[SECRET_SIZE];
};



#endif
