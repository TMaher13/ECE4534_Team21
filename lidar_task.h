#ifndef LIDAR_TASK
#define LIDAR_TASK

void *lidarThread(void *arg0);

int createLidarThread(int threadStackSize, int prio);

#endif
