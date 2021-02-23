#ifndef VERSION2
#define VERSION2

void *version2Thread(void *arg0);

int createVersion2Thread(int threadStackSize, int prio);

#include <version2.c>

#endif
