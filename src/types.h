#ifndef TYPES_H
#define TYPES_H

#include <genesis.h>

typedef struct {
  s16 x;
  s16 y;
  s16 speed;
  s16 gravity;
  Sprite *sprite;
} GameObject;

typedef struct {
  u8 m;
  u8 s;
  u8 ms;
} Time;

#endif  // TYPES_H
