#include <stddef.h>

#ifndef TIMER500
#define TIMER500

void timer500Init();

void timer500Callback(Timer_Handle myHandle, int_fast16_t status);

#include <timer500.c>


#endif
