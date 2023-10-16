#include <genesis.h>
#include <resources.h>

u16 ind = TILE_USER_INDEX;
float fg_offset = 0;

Sprite *player;
u16 player_x = 120;
u16 player_y = 80;
// u16 key_prev = 0;

#define ANIM_STILL 0
#define ANIM_IDLE 1
#define ANIM_WALK 2
#define ANIM_UPPER 3

static void handleInput()
{
    u16 key = JOY_readJoypad(JOY_1);
    u8 speed = 1;
    bool isNotBeatingUpper =
        player->animInd != ANIM_UPPER ||
        player->animInd == ANIM_UPPER && player->frameInd > 5;
    bool isWalking = key & BUTTON_LEFT || key & BUTTON_RIGHT || key & BUTTON_UP || key & BUTTON_DOWN;

    if (key & BUTTON_LEFT && isNotBeatingUpper)
    {
        player_x -= speed;
        fg_offset += 0.3;
        SPR_setHFlip(player, FALSE);
    }
    else if (key & BUTTON_RIGHT && isNotBeatingUpper)
    {
        player_x += speed;
        fg_offset -= 0.3;
        SPR_setHFlip(player, TRUE);
    }
    if (key & BUTTON_UP && isNotBeatingUpper && player_y > 70)
    {
        player_y -= speed;
    }
    else if (key & BUTTON_DOWN && isNotBeatingUpper && player_y < 100)
    {
        player_y += speed;
    }

    // animations
    if (isNotBeatingUpper)
    {
        SPR_setAnim(player, isWalking ? ANIM_WALK : ANIM_IDLE);
    }
    if (key & BUTTON_A)
    {
        SPR_setAnim(player, ANIM_UPPER);
    }

    SPR_setPosition(player, player_x, player_y);
    // foreground
    VDP_setHorizontalScroll(BG_A, fg_offset);
}


int main()
{
    PAL_setPalette(PAL0, bg1.palette->data, DMA);
    VDP_drawImageEx(BG_B, &bg1, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, ind), 0, 0, FALSE, TRUE);
    ind += bg1.tileset->numTile;

    PAL_setPalette(PAL1, fg1.palette->data, DMA);
    VDP_drawImageEx(BG_A, &fg1, TILE_ATTR_FULL(PAL1, 0, FALSE, FALSE, ind), 0, -2, FALSE, TRUE);
    ind += fg1.tileset->numTile;

    VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);

    SPR_init();
    PAL_setPalette(PAL2, my_sprite.palette->data, DMA);
    player = SPR_addSprite(&my_sprite, player_x, player_y, TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    SPR_setAnim(player, ANIM_WALK);

    while (1)
    {
        VDP_drawText("Demo by pzinovev", 0, 0);
        handleInput();

        SPR_update();
        SYS_doVBlankProcess();
    }
    return (0);
}
