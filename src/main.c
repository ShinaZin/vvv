#include <genesis.h>
#include <resources.h>

#include "types.h"
#include "helpers.h"

u16 tileIdx = TILE_USER_INDEX;

GameObject player = {
  .x = 120,
  .y = 100,
  .speed = 3,
  .gravity = 4
};

Time currentTime = {.m = 0, .s = 0, .ms = 0};
Time bestTime;

#define RECTS_COUNT 6

GameObject rects[RECTS_COUNT];

#define ANIM_IDLE 0
#define ANIM_WALK 1
#define ANIM_HURT 2

#define LINE_PADDING 45
#define LINE_TOP_Y 0 + LINE_PADDING
#define LINE_BOTTOM_Y 200 - LINE_PADDING

#define TIMER_SCORE 1

typedef u8 Scene;
#define SCENE_MAIN 0
#define SCENE_PAUSE 1
#define SCENE_GAME_OVER 2

Scene scene = SCENE_MAIN;

Time getCurrentTime() {
  u32 ticks = getTimer(TIMER_SCORE, FALSE);
  u16 msecs = ticks / (SUBTICKPERSECOND / 1000);
  u8 seconds = ticks / SUBTICKPERSECOND;
  u8 minutes = seconds / 60;
  if (minutes == 1) {
    getTimer(TIMER_SCORE, TRUE);
  }
  return (Time){currentTime.m + minutes, seconds % 60, msecs % 1000};
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
        rect->x += getRandom(20, 150);
      } else if (isLeftFromScreen) {
        rect->x += getRandom(-150, -20);
      }
      rect->speed *= -1;
    }
    SPR_setPosition(rect->sprite, rect->x, rect->y);
  }
}

static void handleTimeScore() {
  currentTime = getCurrentTime();
  Time* best = getTimeAsMs(&currentTime) > getTimeAsMs(&bestTime) ? &currentTime : &bestTime;
  VDP_drawTextBG(BG_A, getTimeAsString(&currentTime, "Time: "), 1, 2);
  VDP_drawTextBG(BG_A, getTimeAsString(best, "Best: "), 24, 2);
}

static void handleCollisions() {
  foreach (GameObject *rect, rects) {
    if (isColliding(rect, &player, 7)) {
      scene = SCENE_GAME_OVER;
    }
  }
}


void sceneMain() {
  float bgOffset = 0;
  
  PAL_setPalette(PAL1, i_bg1.palette->data, DMA);
  VDP_drawImageEx(BG_B, &i_bg1,
                  TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE, tileIdx + 1), 0, 0,
                  FALSE, TRUE);

  VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
  int baseline = TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE, tileIdx + i_bg1.tileset->numTile + 1);
  for (u8 tileX = 0; tileX < 40; tileX++) {
    VDP_drawImageEx(BG_A, &i_line_h, baseline, tileX, 5, FALSE, TRUE);
    VDP_drawImageEx(BG_A, &i_line_h, baseline, tileX, 22, FALSE, TRUE);
  }

  SPR_init();
  player.x = 120;
  player.y = 100;
  player.gravity = 4;
  player.sprite = SPR_addSprite(&s_player, player.x, player.y,
                         TILE_ATTR(PAL1, FALSE, FALSE, FALSE));
  foreach (GameObject *rect, rects) {
    rect->x = getRandom(-240, 0);
    rect->y = getRandom(LINE_TOP_Y, LINE_BOTTOM_Y);
    rect->speed = 3; // getRandomSpeed(2, 3);
    rect->sprite = SPR_addSprite(&s_rect, rect->x, rect->y,
                                 TILE_ATTR(PAL1, FALSE, FALSE, FALSE));
  }

  XGM_startPlay(music_main);
  startTimer(TIMER_SCORE);

  // XGM_setPCM(64, sfx_blip, sizeof(sfx_blip));

  while (scene == SCENE_MAIN) {
    handleInput();
    handleGravity();
    handleRects();
    handleCollisions();
    handleTimeScore();
    // handleBackground()
    bgOffset += 2;
    VDP_setVerticalScroll(BG_B, -bgOffset);
    // TODO drawLines on BG_A
    
    if (!XGM_isPlaying()) {
      XGM_startPlay(music_main);
    }

    SPR_update();
    SYS_doVBlankProcess();
  }
}

void sceneGameOver() {
  if (scene == SCENE_GAME_OVER) {
    XGM_stopPlay();
    SPR_setAnim(player.sprite, ANIM_HURT);
    SPR_update();
    if (getTimeAsMs(&currentTime) > getTimeAsMs(&bestTime)) {
      bestTime = currentTime;
    }
    VDP_drawTextBG(BG_A, "GAME OVER", 14, 26);
    waitMs(1000);
    VDP_clearPlane(BG_A, TRUE);
    scene = SCENE_MAIN;
  }
}

int main() {
  while (1) {
    sceneMain();
    sceneGameOver();
  }
  return (0);
}
