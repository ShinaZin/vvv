#include <genesis.h>

int main()
{
    VDP_drawText("Hi, SEGA", 15, 13);

    while (1)
    {
        SYS_doVBlankProcess();
    }
    return (0);
}
