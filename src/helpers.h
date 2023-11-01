#ifndef HELPERS_H
#define HELPERS_H

#include <genesis.h>

#include "types.h"

#define foreach(item, array)                                            \
  for (int keep = 1, count = 0, size = sizeof(array) / sizeof *(array); \
       keep && count != size; keep = !keep, count++)                    \
    for (item = (array) + count; keep; keep = !keep)

s16 getRandom(s16 mini, s16 maxi);

s16 getRandomSpeed(s16 min, s16 max);

/** Convert `time` to miliseconds */
u16 getTimeAsMs(const Time *time);
/** Convert `time` to string and add some `prefix` */
char *getTimeAsString(const Time *time, const char *prefix);

bool isColliding(GameObject *a, GameObject *b, s8 padding);

#endif  // HELPERS_H
