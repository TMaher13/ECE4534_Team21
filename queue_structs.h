#ifndef QUEUE_STRUCTS
#define QUEUE_STRUCTS

#include <stdlib.h>
#include <stdint.h>


#define TOPIC_SIZE                  64
#define PAYLOAD_SIZE                256

#define CAMERA_PACKET_SIZE          64
#define IMAGE_SIZE                  128

#define I2C_SLAVE_ADDRESS           0x12

/*  Lidar codes and values  */
#define LIDAR_REQUEST_SIZE          2
#define LIDAR_RD_SIZE               7
#define LIDAR_HEALTH_SIZE           3
#define LIDAR_SCAN_SIZE             5
#define LIDAR_INFO_SIZE             20

#define MQTT_MESSAGE                1

#define HEALTH_REQUEST              0
#define INFO_REQUEST                1
#define SCAN_REQUEST                2
#define STOP_REQUEST                3



struct lidarQueueStruct {
    uint_least8_t messageType;
    uint_least8_t value;
};

struct cameraQueueStruct {

    //char img[IMAGE_SIZE*IMAGE_SIZE];
};

struct navQueueStruct {

    uint_least8_t   messageType;
    uint32_t        tickTime;

    //char img[IMAGE_SIZE*IMAGE_SIZE];
};


struct mqttQueueStruct {

    char topic[TOPIC_SIZE];
    char payload[PAYLOAD_SIZE];

};



#endif
