#include "./build/bank2.h"
#include "main.h"

// LEVEL ROUTINES
void endLevel(void)
{
    // END LEVEL LOOP
    PSGPlayNoRepeat(exit_psg);
    for (;;)
    {
        SMS_waitForVBlank();
        SMS_copySpritestoSAT();
        PSGFrame();
        if (PSGGetStatus() == PSG_PLAYING)
        {
            continue;
        }
        else
        {
            PSGStop();
            exit = 1;
            break;
        }
    }
}

void deathLoop(void)
{
    // DEATH LOOP
    PSGPlayNoRepeat(death_psg);
    for (;;)
    {
        SMS_waitForVBlank();
        SMS_copySpritestoSAT();
        PSGFrame();
        if (PSGGetStatus() == PSG_PLAYING)
        {
            continue;
        }
        else
        {
            PSGStop();
            // set player state to NOT MOVING
            player.isMoving = 0;
            // reset step
            step = 0;
            // reset player sprite
            player.sprite = 0;
            break;
        }
    }
}

void levelLoop(void (*initLevel)(void))
{
    PSGPlay(wtm_psg);
    initLevel();
    unsigned char collisionCode;
    while (!exit)
    {
        frameCounter++;
        // tile animation happens every N frames
        if ((levelCopy[speedPowerupPos[0] + speedPowerupPos[1] * 32] == POWERUP_SPEED) && (frameCounter % TILE_ANIMATION_FRAME == 0))
        {
            // speed power up
            speedPowerupAnimation = !speedPowerupAnimation;
            SMS_setTileatXY(speedPowerupPos[0], speedPowerupPos[1], speedPowerupTiles[speedPowerupAnimation] | TILE_USE_SPRITE_PALETTE);
        }
        ks = SMS_getKeysStatus();
        // if player is NOT moving
        if (player.isMoving == 0)
        {
            // if so, check the collision against walls in that direction
            dir = getDirection();
            collisionCode = levelCollision(dir);
            if ((dir != UNKNOWN) && (collisionCode != WALL))
            {
                // if there's no wall, set player as MOVING
                player.isMoving = 1;
                // store the direction
                player.dir = dir;
                // increase the speed in case of speed powerup
                if (collisionCode == POWERUP_SPEED)
                {
                    player.speed = 2;  // set speed to 2
                    player.spedUp = 1; // set player status to sped up
                    // remove power-up from level (substitute it with terrain tile)
                    SMS_setTileatXY(speedPowerupPos[0], speedPowerupPos[1], 1);
                    levelCopy[speedPowerupPos[0] + speedPowerupPos[1] * 32] = TERRAIN;
                }
            }
        }
        // if player is sped up
        if ((player.spedUp) && (speedPowerupCounter < MAX_SPEED_POWERUP))
        {
            speedPowerupCounter++; // increase the speed frame counter
            PSGFrame();            // play another frame of music (to speed the music up)
        }
        else
        {
            speedPowerupCounter = 0; // reset speed powerup frame counter
            player.speed = 1;         // reset player speed
            player.spedUp = 0;        // set player status to normal
        }
        // if player is MOVING
        if (player.isMoving == 1)
        {
            // move it n step towards the right direction
            movePlayer(player.dir);
            step += player.speed;
        }
        // if player has reached the end of a tile
        if (step == MAXSTEPS)
        {
            // set player state to NOT MOVING
            player.isMoving = 0;
            // reset step
            step = 0;
            // animate the sprite (values between 0 and 1)
            player.sprite = !player.sprite;
        }
        SMS_initSprites();
        drawPlayer();
        // draw and move the monsters
        for (size_t i = 0; i < activeMonsters; i++)
        {
            drawMonster(&monsters[i]);
            moveMonster(&monsters[i]);
        }
        drawExit();
        SMS_waitForVBlank();
        SMS_copySpritestoSAT();
        PSGFrame();
        if ((player.pos[0] / 8 == exitPosTile[0]) & (player.pos[1] / 8 == exitPosTile[1]))
        {
            endLevel();
            break;
        }
        if (monsterCollision())
        {
            deathLoop();
            levelLoop(initLevel);
        }
    }
}

// LEVEL DEFINITIONS
void level1(void)
{
    SMS_loadTiles(level1__tiles__bin, BG_TILES, level1__tiles__bin_size);
    SMS_loadTileMap(0, 0, level1__tilemap__bin, level1__tilemap__bin_size);

    // copy level1 to levelCopy
    memcpy(levelCopy, level1_objects, LEVEL_SIZE * sizeof(level1_objects[0]));

    // monster array
    activeMonsters = 4;

    monsters[0].name = REXY;
    monsters[0].dir = RIGHT;
    monsters[0].speed = 1;
    monsters[0].sprite = 0;
    monsters[0].pos[0] = 8;
    monsters[0].pos[1] = 88;
    monsters[0].limits[0] = 8;   // xmin limit
    monsters[0].limits[1] = 216; // xmax limit

    monsters[1].name = POCCO;
    monsters[1].dir = RIGHT;
    monsters[1].speed = 4;
    monsters[1].sprite = 0;
    monsters[1].pos[0] = 120;
    monsters[1].pos[1] = 144;
    monsters[1].limits[0] = 8;   // xmin limit
    monsters[1].limits[1] = 232; // xmax limit

    monsters[2].name = REXY;
    monsters[2].dir = LEFT;
    monsters[2].speed = 2;
    monsters[2].sprite = 0;
    monsters[2].pos[0] = 8;
    monsters[2].pos[1] = 88;
    monsters[2].limits[0] = 8;   // xmin limit
    monsters[2].limits[1] = 216; // xmax limit

    monsters[3].name = REXY;
    monsters[3].dir = RIGHT;
    monsters[3].speed = 1;
    monsters[3].sprite = 0;
    monsters[3].pos[0] = 176;
    monsters[3].pos[1] = 16;
    monsters[3].limits[0] = 160; // xmin limit
    monsters[3].limits[1] = 200; // xmax limit

    speedPowerupPos[0] = 30;
    speedPowerupPos[1] = 3;
    // set the power-up tile to use the sprite palette
    SMS_setTileatXY(speedPowerupPos[0], speedPowerupPos[1], speedPowerupTiles[speedPowerupAnimation] | TILE_USE_SPRITE_PALETTE);
    speedPowerupCounter = 0; // frame counter for the speed powerup
    // all sprite coordinates must refer to the top-left pixel within a tile
    
    // main character
    player.speed = 1;   // MUST BE A FACTOR OF 8! (e.g. 1,2,4,8)
    player.sprite = 0;
    player.spedUp = 0;  // affected by speed powerup (1: sped up; 0: normal speed)
    player.pos[0] = 8;
    player.pos[1] = 8;

    // exit door
    exitPosPixel[0] = 240;                // position in px
    exitPosPixel[1] = 176;                // position in px
    exitPosTile[0] = exitPosPixel[0] / 8; // position in tile
    exitPosTile[1] = exitPosPixel[1] / 8; // position in tile
}

void level2(void)
{
    SMS_loadTiles(level2__tiles__bin, BG_TILES, level2__tiles__bin_size);
    SMS_loadTileMap(0, 0, level2__tilemap__bin, level2__tilemap__bin_size);

    // copy level1 to levelCopy
    memcpy(levelCopy, level1_objects, LEVEL_SIZE * sizeof(level1_objects[0]));

    // monster array
    activeMonsters = 1;

    // monster array
    monsters[0].name = REXY;
    monsters[0].dir = RIGHT;
    monsters[0].speed = 1;
    monsters[0].sprite = 0;
    monsters[0].pos[0] = 8;
    monsters[0].pos[1] = 88;
    monsters[0].limits[0] = 8;   // xmin limit
    monsters[0].limits[1] = 216; // xmax limit

    speedPowerupCounter = 0; // frame counter for the speed powerup
    // all sprite coordinates must refer to the top-left pixel within a tile
    
    // main character
    player.speed = 1;   // MUST BE A FACTOR OF 8! (e.g. 1,2,4,8)
    player.sprite = 0;
    player.spedUp = 0;  // affected by speed powerup (1: sped up; 0: normal speed)
    player.pos[0] = 8;
    player.pos[1] = 8;

    // exit door
    exitPosPixel[0] = 240;                // position in px
    exitPosPixel[1] = 176;                // position in px
    exitPosTile[0] = exitPosPixel[0] / 8; // position in tile
    exitPosTile[1] = exitPosPixel[1] / 8; // position in tile
}
