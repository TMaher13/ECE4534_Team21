#ifndef CAMERA_TASK
#define CAMERA_TASK

void *cameraThread(void *arg0);

int createCameraThread(int threadStackSize, int prio);

#endif
