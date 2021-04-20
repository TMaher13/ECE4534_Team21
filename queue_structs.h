#ifndef QUEUE_STRUCTS
#define QUEUE_STRUCTS

#include <stdlib.h>
#include <stdint.h>


#define TOPIC_SIZE                  64
#define PAYLOAD_SIZE                256

#define LIDAR_DEGREES               360


#define IMG_TARGET                  0
#define IMG_OBSTACLE                1


/*  Lidar codes and values  */
#define LIDAR_REQUEST_SIZE          2
#define LIDAR_RD_SIZE               7
#define LIDAR_HEALTH_SIZE           3
#define LIDAR_SCAN_SIZE             5
#define LIDAR_INFO_SIZE             20

#define MQTT_MESSAGE                1
#define TIMER_MESSAGE               2

#define LIDAR_MESSAGE               0
#define CAMERA_MESSAGE              2
#define REQUEST_MESSAGE             3

#define HEALTH_REQUEST              0
#define INFO_REQUEST                1
#define SCAN_REQUEST                2
#define STOP_REQUEST                3


#define PATH_REQUEST                0
#define DIST_REQUEST                1
#define CAMERA_DET                  2


#define SIMPLE_DIST_REQUEST         0
#define ROVER_PATH_REQUEST          1

#define IN_FRONT                    1
#define ONLY_OBJ                    2
#define IN_BACK                     3


struct imageDetection {
    int_least8_t object;
    int_least16_t angle;
    uint_least8_t location;
};

struct lidarQueueStruct {
    uint_least8_t messageType;
    uint_least8_t value;
};

struct navQueueStruct {
    uint_least8_t   messageType;
    uint32_t        tickTime;

    uint_least8_t request;
    uint16_t scanData[LIDAR_DEGREES];
    struct imageDetection imgDet;
};


struct mqttQueueStruct {
    char topic[TOPIC_SIZE];
    char payload[PAYLOAD_SIZE];
};



#endif
