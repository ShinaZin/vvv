#include <genesis.h>
#include <resources.h>

u16 tileIdx = TILE_USER_INDEX;
float bgOffset = 0;

typedef struct {
  s16 x;
  s16 y;
  s16 speed;
  s16 gravity;
  Sprite *sprite;
} GameObject;

GameObject player = {
  .x = 120,
  .y = 100,
  .speed = 3,
  .gravity = 4
};

u8 timeMinutes = 0;

#define RECTS_COUNT 4

GameObject rects[RECTS_COUNT];

#define foreach(item, array)                                            \
  for (int keep = 1, count = 0, size = sizeof(array) / sizeof *(array); \
       keep && count != size; keep = !keep, count++)                    \
    for (item = (array) + count; keep; keep = !keep)

#define ANIM_IDLE 0
#define ANIM_WALK 1
#define ANIM_HURT 2

#define LINE_PADDING 45
#define LINE_TOP_Y 0 + LINE_PADDING
#define LINE_BOTTOM_Y 200 - LINE_PADDING

#define TIMER_SCORE 1

s16 getRandom(s16 mini, s16 maxi) {
  if (mini > maxi)
    SWAP_s16(mini, maxi);
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

static void handleInput() {
  u16 key = JOY_readJoypad(JOY_1);
  bool isWalking = key & BUTTON_LEFT || key & BUTTON_RIGHT;

  if (key & BUTTON_LEFT) {
    player.x -= player.speed;
    SPR_setHFlip(player.sprite, TRUE);
  } else if (key & BUTTON_RIGHT) {
    player.x += player.speed;
    SPR_setHFlip(player.sprite, FALSE);
  }
  if (player.x > screenWidth)
    player.x = 0;
  else if (player.x < 0 - s_player.w)
    player.x = screenWidth;
  // animations
  SPR_setAnim(player.sprite, isWalking ? ANIM_WALK : ANIM_IDLE);
  SPR_setPosition(player.sprite, player.x, player.y);
}

static void handleGravity() {
  bool isInverting = player.y > LINE_BOTTOM_Y || player.y < LINE_TOP_Y;
  if (isInverting) {
    player.gravity *= -1;
    // XGM_startPlayPCM(64, 15, SOUND_PCM_CH2);
    SPR_setVFlip(player.sprite, player.gravity < 0);
  }
  player.y += player.gravity;
}

static void handleRects() {
  foreach (GameObject *rect, rects) {
    rect->x += rect->speed;
    bool isRightFromScreen = rect->x > screenWidth && rect->speed > 0;
    bool isLeftFromScreen = rect->x < 0 - s_rect.w && rect->speed < 0;
    if (isRightFromScreen || isLeftFromScreen) {
      rect->y = getRandom(LINE_TOP_Y, LINE_BOTTOM_Y);
      if (isRightFromScreen) {
        rect->x += getRandom(20, 100);
      } else if (isLeftFromScreen) {
        rect->x += getRandom(-100, -20);
      }
      rect->speed *= -1;
    }
    SPR_setPosition(rect->sprite, rect->x, rect->y);
  }
}

static void handleCollisions() {
  foreach (GameObject *rect, rects) {
    if (isColliding(rect, &player, 7)) {
      XGM_stopPlay();
      SPR_setAnim(player.sprite, ANIM_HURT);
      SPR_update();
      VDP_drawTextBG(BG_A, "GAME OVER", 14, 26);
      waitMs(1000);
      SYS_reset();
    }
  }
}

static char *getCurrentTimeScore() {
  static char stringified[20];

  u32 ticks = getTimer(TIMER_SCORE, FALSE);
  u16 msecs = ticks / (SUBTICKPERSECOND / 1000);
  u8 seconds = ticks / SUBTICKPERSECOND;
  u8 minutes = seconds / 60;
  if (minutes == 1) {
    timeMinutes += 1;
    getTimer(TIMER_SCORE, TRUE);
  }

  sprintf(stringified, "Time: %02d:%02d:%03d", timeMinutes, seconds % 60,
          msecs % 1000);
  return stringified;
}

int main() {
  PAL_setPalette(PAL1, i_bg1.palette->data, DMA);
  VDP_drawImageEx(BG_B, &i_bg1,
                  TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE, tileIdx + 1), 0, 0,
                  FALSE, TRUE);

  VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);

  SPR_init();
  player.x = 120;
  player.y = 100;
  player.gravity = 4;
  player.sprite = SPR_addSprite(&s_player, player.x, player.y,
                         TILE_ATTR(PAL1, FALSE, FALSE, FALSE));
  foreach (GameObject *rect, rects) {
    rect->x = 0;
    rect->y = getRandom(LINE_TOP_Y, LINE_BOTTOM_Y);
    rect->speed = 3; // getRandomSpeed(2, 3);
    rect->sprite = SPR_addSprite(&s_rect, rect->x, rect->y,
                                 TILE_ATTR(PAL1, FALSE, FALSE, FALSE));
  }

  XGM_startPlay(music_main);
  startTimer(TIMER_SCORE);

  // XGM_setPCM(64, sfx_blip, sizeof(sfx_blip));

  while (1) {
    handleInput();
    handleGravity();
    handleRects();
    handleCollisions();
    // bg scroll
    bgOffset += 2;
    VDP_setVerticalScroll(BG_B, -bgOffset);
    VDP_drawTextBG(BG_A, getCurrentTimeScore(), 1, 2);

    if (!XGM_isPlaying()) {
      XGM_startPlay(music_main);
    }

    SPR_update();
    SYS_doVBlankProcess();
  }
  return (0);
}
