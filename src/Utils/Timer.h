#pragma once

#include <ctime>

#define TIMER_INIT() (clock())

/**
 find the difference in milliseconds with TIMER_DIFF(clock_t timer)
 */
#define TIMER_DIFF(t) (clock() - t)
