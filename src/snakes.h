#ifndef __SNAKES_H__
#define __SNAKES_H__

#include <Arduino.h>

struct TailSegment
{
    uint8_t x;
    uint8_t y;
};

extern TailSegment HILBERT_CURVE[];
extern const int HILBERT_CURVE_SIZE;

#endif // __SNAKES_H__