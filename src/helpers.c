#include "types.h"

s16 getRandom(s16 mini, s16 maxi) {
  if (mini > maxi) SWAP_s16(mini, maxi);
  return mini + (random() % (maxi - mini + 1));
}

s16 getRandomSpeed(s16 min, s16 max) {
  s8 sign = getRandom(0, 10) > 5 ? 1 : -1;
  return getRandom(min, max) * sign;
}

bool isColliding(GameObject *a, GameObject *b, s8 padding) {
  const s16 aX = a->x + padding;
  const s16 aY = a->y + padding;
  const s16 aW = a->sprite->definition->w + aX - padding;
  const s16 aH = a->sprite->definition->h + aY - padding;

  const s16 bX = b->x + padding;
  const s16 bY = b->y + padding;
  const s16 bW = b->sprite->definition->w + bX - padding;
  const s16 bH = b->sprite->definition->h + bY - padding;
  return (aX < bW && aW > bX && aY < bH && aH > bY);
}
