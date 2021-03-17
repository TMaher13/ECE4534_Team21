#include <stddef.h>

#ifndef TIMER70
#define TIMER70

void timer70Init();

void timer70Callback(Timer_Handle myHandle, int_fast16_t status);

#include <timer70.c>

#endif
