#ifndef LIDAR_TASK
#define LIDAR_TASK

#include <stdint.h>

#define FP_ANGLE_FRACTION_BITS  6
#define FP_DIST_FRACTION_BITS   2

struct lidarData {
    uint16_t angle;
    uint16_t dist;
};

void *lidarThread(void *arg0);

int createLidarThread(int threadStackSize, int prio);

#endif
