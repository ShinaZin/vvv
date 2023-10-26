#include <genesis.h>
#include <resources.h>

u16 tileIdx = TILE_USER_INDEX;
float bgOffset = 0;

Sprite *player;
s16 playerX = 120;
s16 playerY = 100;
u8 playerSpeed = 3;
s16 playerGravity = 4;

u8 timeMinutes = 0;

#define RECTS_COUNT 4

typedef struct {
  s16 x;
  s16 y;
  s16 speed;
  Sprite *sprite;
} Rect;

Rect rects[RECTS_COUNT];

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

static void handleInput() {
  u16 key = JOY_readJoypad(JOY_1);
  bool isWalking = key & BUTTON_LEFT || key & BUTTON_RIGHT;

  if (key & BUTTON_LEFT) {
    playerX -= playerSpeed;
    SPR_setHFlip(player, TRUE);
  } else if (key & BUTTON_RIGHT) {
    playerX += playerSpeed;
    SPR_setHFlip(player, FALSE);
  }
  if (playerX > screenWidth)
    playerX = 0;
  else if (playerX < 0 - s_player.w)
    playerX = screenWidth;
  // animations
  SPR_setAnim(player, isWalking ? ANIM_WALK : ANIM_IDLE);
  SPR_setPosition(player, playerX, playerY);
}

static void handleGravity() {
  bool isInverting = playerY > LINE_BOTTOM_Y || playerY < LINE_TOP_Y;
  if (isInverting) {
    playerGravity *= -1;
    // XGM_startPlayPCM(64, 15, SOUND_PCM_CH2);
    SPR_setVFlip(player, playerGravity < 0);
  }
  playerY += playerGravity;
}

static void handleRects() {
  foreach (Rect *rect, rects) {
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
  player = SPR_addSprite(&s_player, playerX, playerY,
                         TILE_ATTR(PAL1, FALSE, FALSE, FALSE));
  foreach (Rect *rect, rects) {
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
