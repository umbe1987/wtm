#include "./lib/SMSlib.h" // we're including the library with the functions we will use
#include "./lib/PSGlib.h"
#include "./build/bank2.h"
#include <string.h>
#include "main.h"
#include "level.c"

unsigned char frameCounter = 0;              // keep track of frame number (e.g. for animation)

// DRAW ROUTINES
void drawPlayer(void)
{
    SMS_addSprite(player.pos[0], player.pos[1], PLAYER_TILES + player.sprite);
}

void drawExit(void)
{
    SMS_addSprite(exitPosPixel[0], exitPosPixel[1], EXIT_TILES);
}

void drawRexy(struct Monster *rexy)
{
    if (rexy->dir == RIGHT)
    {
        SMS_addSprite(rexy->pos[0], rexy->pos[1], REXY_TILES_RIGHT + rexy->sprite);
    }
    if (rexy->dir == LEFT)
    {
        SMS_addSprite(rexy->pos[0], rexy->pos[1], REXY_TILES_LEFT + rexy->sprite);
    }
}

void drawPocco(struct Monster *pocco)
{
    if (pocco->sprite)
    {
        SMS_addSprite(pocco->pos[0], pocco->pos[1], POCCO_TILES);
        SMS_addSprite(pocco->pos[0] + 8, pocco->pos[1], POCCO_TILES + 1);
    }
    else
    {
        SMS_addSprite(pocco->pos[0], pocco->pos[1], POCCO_TILES + 2);
        SMS_addSprite(pocco->pos[0] + 8, pocco->pos[1], POCCO_TILES + 3);
    }
}

void drawMonster(struct Monster *monster)
{
    switch (monster->name)
    {
    case REXY:
        drawRexy(monster);
        break;

    case POCCO:
        drawPocco(monster);
        break;

    default:
        break;
    }
}

// MOVE ROUTINES
void movePlayer(enum Direction dir)
{
    if (dir == UP)
    {
        player.pos[1] -= player.speed;
    }
    if (dir == DOWN)
    {
        player.pos[1] += player.speed;
    }
    if (dir == LEFT)
    {
        player.pos[0] -= player.speed;
    }
    if (dir == RIGHT)
    {
        player.pos[0] += player.speed;
    }
}

void moveRexy(struct Monster *rexy)
{
    if (rexy->pos[0] == rexy->limits[0])
    {
        rexy->dir = RIGHT;
    }
    if (rexy->pos[0] == rexy->limits[1])
    {
        rexy->dir = LEFT;
    }
    if (rexy->dir == RIGHT)
    {
        rexy->pos[0] += rexy->speed;
    }
    if (rexy->dir == LEFT)
    {
        rexy->pos[0] -= rexy->speed;
    }
    // change monster animation frame
    if (rexy->pos[0] % (8 * rexy->speed) == 0)
    {
        rexy->sprite = !rexy->sprite;
    }
}

void movePocco(struct Monster *pocco)
{
    if (pocco->pos[0] == pocco->limits[0])
    {
        pocco->dir = RIGHT;
    }
    if (pocco->pos[0] == pocco->limits[1])
    {
        pocco->dir = LEFT;
    }
    if (pocco->dir == RIGHT)
    {
        pocco->pos[0] += pocco->speed;
    }
    if (pocco->dir == LEFT)
    {
        pocco->pos[0] -= pocco->speed;
    }
    // change monster animation frame
    if (pocco->pos[0] % (8 * pocco->speed) == 0)
    {
        pocco->sprite = !pocco->sprite;
    }
}

void moveMonster(struct Monster *monster)
{
    switch (monster->name)
    {
    case REXY:
        moveRexy(monster);
        break;

    case POCCO:
        movePocco(monster);
        break;

    default:
        break;
    }
}

// COLLISION ROUTINES

// Check if player hits a monster. Return 1 in case it does, 0 otherwise
// use tiles instead of pixels
unsigned char monsterCollision(void)
{
    unsigned char i;
    unsigned char playerTileX = player.pos[0] / 8;
    unsigned char playerTileY = player.pos[1] / 8;

    for (i = 0; i < activeMonsters; i++)
    {
        if ((playerTileX == monsters[i].pos[0] / 8) & (playerTileY == monsters[i].pos[1] / 8))
        {
            return 1; // collision!
        }
    }
    return 0; // no collision
}

// return the index of the next tile in a given direction
unsigned int nextMove(enum Direction dir)
{
    // player index in tile coordinates
    unsigned char x = player.pos[0] / 8;
    unsigned char y = player.pos[1] / 8;
    // player tile index within level
    unsigned int ix = x + y * 32;
    if (dir == UP)
    {
        return ix - 32;
    }
    if (dir == DOWN)
    {
        return ix + 32;
    }
    if (dir == LEFT)
    {
        return ix - 1;
    }
    if (dir == RIGHT)
    {
        return ix + 1;
    }
    return ix; // in case direction is UNKNOW
}

enum Direction getDirection(void)
{
    if (ks & PORT_A_KEY_UP)
    {
        return UP;
    }
    if (ks & PORT_A_KEY_DOWN)
    {
        return DOWN;
    }
    if (ks & PORT_A_KEY_LEFT)
    {
        return LEFT;
    }
    if (ks & PORT_A_KEY_RIGHT)
    {
        return RIGHT;
    }
    return UNKNOWN;
}

// MISC
void loadAssets(void)
{
    SMS_loadBGPalette(maze__palette__bin);
    SMS_loadTiles(sprites__tiles__bin, SPRITE_TILES, sprites__tiles__bin_size);
    SMS_loadSpritePalette(sprites__palette__bin);
}

// MAIN GAME LOOP
void main(void)
{
    loadAssets();
    SMS_displayOn();
    exit = 0; // reset exit
    levelLoop(level1);
    exit = 0; // reset exit
    levelLoop(level2);
}
