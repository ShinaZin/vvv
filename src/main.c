#include <genesis.h>
#include <resources.h>

u16 tileIdx = TILE_USER_INDEX;
float bgOffset = 0;

Sprite *player;
u16 playerX = 120;
u16 playerY = 100;
s16 playerGravity = 4;

u8 timeMinutes = 0;

#define ANIM_IDLE 0
#define ANIM_WALK 1
#define ANIM_HURT 2

#define LINE_PADDING 45
#define LINE_TOP_Y 0 + LINE_PADDING
#define LINE_BOTTOM_Y 200 - LINE_PADDING

#define TIMER_SCORE 1

static void handleInput() {
  u16 key = JOY_readJoypad(JOY_1);
  u8 speed = 2;
  bool isWalking = key & BUTTON_LEFT || key & BUTTON_RIGHT;

  if (key & BUTTON_LEFT) {
    playerX -= speed;
    SPR_setHFlip(player, TRUE);
  } else if (key & BUTTON_RIGHT) {
    playerX += speed;
    SPR_setHFlip(player, FALSE);
  }
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

  XGM_startPlay(music_main);
  startTimer(TIMER_SCORE);

  // XGM_setPCM(64, sfx_blip, sizeof(sfx_blip));

  while (1) {
    handleInput();
    handleGravity();
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
