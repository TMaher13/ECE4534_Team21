#ifndef NAV_TASK
#define NAV_TASK

void *navigationThread(void *arg0);

int createNavigationThread(int threadStackSize, int prio);

#endif
