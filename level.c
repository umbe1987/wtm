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
            // set player state to NOT MOVING
            player.isMoving = 0;
            // reset step
            step = 0;
            // reset player sprite
            player.sprite = 0;
            end = 1;
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

// REMEMBER! we can only have MAX_POWERUPS in a level
// Trying to add more will have unexpected effects on screen
void getPowerups(struct PowerUp *pu)
{
    activePowerUps = 0; // reset number of powerups using a global variable
    for (unsigned int i = 0; i < LEVEL_SIZE; i++)
    {
        enum LevelObject id = levelCopy[i];
        if (id > EXIT)
        {
            pu[activePowerUps].type = levelCopy[i]; // set type of powerup
            pu[activePowerUps].ix = i;              // store index of powerup in level
            pu[activePowerUps].x = i % 32;          // powerup x coord (in tile)
            pu[activePowerUps].y = i / 32;          // powerup y coord (in tile)
            pu[activePowerUps].counter = 0;
            // power up tile index in tileset
            switch (id)
            {
            case POWERUP_FLY:
                pu[activePowerUps].tiles[0] = 2;
                pu[activePowerUps].tiles[1] = 3;
                break;

            case POWERUP_SPEED:
                pu[activePowerUps].tiles[0] = 4;
                pu[activePowerUps].tiles[1] = 5;

            default:
                break;
            }
            pu[activePowerUps].animation = 0; // 0: first animation; 1: second animation
            pu[activePowerUps].taken = 0;     // 0: not taken; 1: taken (to remove it from the level)
            activePowerUps++;                 // increment powerup counter
            if (activePowerUps == MAX_POWERUPS)
            {
                return;
            }
        }
    }
}

// reset all power up effects from the player
void resetPowerup(void)
{
    powerupCounter = 0; // reset powerup frame counter
    player.speed = 1;   // reset player speed
    player.spedUp = 0;  // set player status to normal
    player.canFly = 0;  // player cannot fly anymore
}

// used to remove the powerup from the level if the player takes it
void removePowerup(void)
{
    // get next player position index (in tile)
    unsigned int ix = nextMove(player.dir);
    // translate into xy coords
    unsigned int x = ix % 32;
    unsigned int y = ix / 32;
    // extract the power up the player is on, and set it to taken
    for (unsigned char i = 0; i < activePowerUps; i++)
    {
        // if the index of the powerup corresponds to the player index
        if (powerups[i].ix == ix)
        {
            // remove it from the level
            SMS_setTileatXY(x, y, TERRAIN); // visually...
            levelCopy[ix] = TERRAIN;        // ...and as level object
            powerups[i].taken = 1;          // set it to taken
        }
    }
}

// react to powerup according to their code
void managePowerup(enum LevelObject code)
{
    switch (code)
    {
    // increase the speed in case of speed powerup
    case POWERUP_SPEED:
        player.speed = 2;  // set speed to 2
        player.spedUp = 1; // set player status to sped up
        break;

    case POWERUP_FLY:
        player.canFly = 1; // set player status to can fly
        break;

    case POWERUP_REVERSE:
        /* code */
        break;

    default:
        return; // return in case of non-powerup
    }
    powerupCounter = 1; // start powerup frame counter (0 means inactive, so we start from 1)
    removePowerup();    // remove powerup from level (substitute it with terrain tile)

    return;
}

void levelLoop(void (*initLevel)(void))
{
    pause = 0;                  // always init in a non-paused state
    SMS_resetPauseRequest();
    PSGPlay(wtm_psg);
    initLevel();
    enum LevelObject collisionCode = UNKNOWN;
    while (!end)
    {
        // if pause was pressed
        if (SMS_queryPauseRequested())
        {
            SMS_resetPauseRequest(); // acknowledge pause press
            pause = !pause;          // change game state to pause

            if (pause)
            {
                PSGSFXPlay(pause_psg, SFX_CHANNEL2);
                PSGSetMusicVolumeAttenuation(15); // silence all PSG channels
            }
            else
            {
                PSGSetMusicVolumeAttenuation(0); // restore PSG channels volume
            }
        }

        frameCounter++;
        // tile animation for powerups
        for (unsigned char i = 0; i < activePowerUps; i++)
        {
            if ((frameCounter % TILE_ANIMATION_FRAME == 0) && (!powerups[i].taken))
            {
                powerups[i].animation = !powerups[i].animation; // 0 -> 1 -> 0 -> ...
                SMS_setTileatXY(powerups[i].x, powerups[i].y, powerups[i].tiles[powerups[i].animation]);
            }
        }
        // tile animation for exit
        if (frameCounter % TILE_ANIMATION_FRAME == 0)
        {
            exit.animation = !exit.animation; // 0 -> 1 -> 0 -> ...
            SMS_setTileatXY(exit.x, exit.y, exit.tiles[exit.animation]);
        }
        
        if (pause)
        {
            SMS_initSprites();
            drawPause();
            SMS_waitForVBlank();
            SMS_copySpritestoSAT();
            PSGSFXFrame();
            continue;
        }

        ks = SMS_getKeysStatus();
        // if player is NOT moving
        if (player.isMoving == 0)
        {
            // check the collision against walls in given direction
            dir = getDirection();
            collisionCode = levelCopy[nextMove(dir)];
            if ((dir != UNKNOWN && collisionCode != WALL) || player.canFly)
            {
                // if there's no wall, set player as MOVING
                player.isMoving = 1;
                // store the direction
                player.dir = dir;
                // handle powerups
                managePowerup(collisionCode);
            }
            if (collisionCode == EXIT)
            {
                endLevel();
            }
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
        // if player is sped up
        if (player.spedUp)
        {
            PSGFrame(); // play another frame of music (to speed the music up)
        }
        // if player can fly
        if (player.canFly)
        {
            if (powerupCounter % 4 == 0)
            {
                // overwrite sprite player with fly sprite each n frames to make it look like it's blinking
                SMS_addSprite(player.pos[0], player.pos[1], FLY_TILES);
            }
        }
        if (powerupCounter > 0)
        {
            powerupCounter++; // increase the speed frame counter
        }
        // cancel powerup effects if the counter reached the maximum
        if (powerupCounter >= MAX_POWERUP_COUNTER)
        {
            resetPowerup();
        }
        drawHUD();
        drawPlayer();
        // draw and move the monsters
        for (unsigned char i = 0; i < activeMonsters; i++)
        {
            drawMonster(&monsters[i]);
            moveMonster(&monsters[i]);
        }
        SMS_waitForVBlank();
        SMS_copySpritestoSAT();
        PSGFrame();
        PSGSFXFrame();
        if (!player.canFly && monsterCollision())
        {
            deathLoop();
            levelLoop(initLevel);
        }
    }
}

// LEVEL DEFINITIONS
void level1(void)
{
    SMS_loadBGPalette(level1__palette__bin);
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
    monsters[0].w = REXYWIDTH;
    monsters[0].h = REXYHEIGHT;
    getBBox(monsters[0].pos[0], monsters[0].pos[1], monsters[0].w, monsters[0].h, &monsters[0].bbox);

    monsters[1].name = POCCO;
    monsters[1].dir = RIGHT;
    monsters[1].speed = 4;
    monsters[1].sprite = 0;
    monsters[1].pos[0] = 120;
    monsters[1].pos[1] = 144;
    monsters[1].limits[0] = 8;   // xmin limit
    monsters[1].limits[1] = 232; // xmax limit
    monsters[1].w = POCCOWIDTH;
    monsters[1].h = POCCOHEIGHT;
    getBBox(monsters[1].pos[0], monsters[1].pos[1], monsters[1].w, monsters[1].h, &monsters[1].bbox);

    monsters[2].name = REXY;
    monsters[2].dir = LEFT;
    monsters[2].speed = 2;
    monsters[2].sprite = 0;
    monsters[2].pos[0] = 8;
    monsters[2].pos[1] = 88;
    monsters[2].limits[0] = 8;   // xmin limit
    monsters[2].limits[1] = 216; // xmax limit
    monsters[2].w = REXYWIDTH;
    monsters[2].h = REXYHEIGHT;
    getBBox(monsters[2].pos[0], monsters[2].pos[1], monsters[2].w, monsters[2].h, &monsters[2].bbox);

    monsters[3].name = REXY;
    monsters[3].dir = RIGHT;
    monsters[3].speed = 1;
    monsters[3].sprite = 0;
    monsters[3].pos[0] = 176;
    monsters[3].pos[1] = 16;
    monsters[3].limits[0] = 160; // xmin limit
    monsters[3].limits[1] = 200; // xmax limit
    monsters[3].w = REXYWIDTH;
    monsters[3].h = REXYHEIGHT;
    getBBox(monsters[3].pos[0], monsters[3].pos[1], monsters[3].w, monsters[3].h, &monsters[3].bbox);

    // powerup array
    getPowerups(powerups);
    powerupCounter = 0; // frame counter for the speed powerup

    // main character
    player.speed = 1; // MUST BE A FACTOR OF 8! (e.g. 1,2,4,8)
    player.sprite = 0;
    player.spedUp = 0; // affected by speed powerup (1: sped up; 0: normal speed)
    player.pos[0] = 8;
    player.pos[1] = 8;
    player.w = PLAYERWIDTH;
    player.h = PLAYERHEIGHT;
    player.canFly = 0; // affected by fly powerup (1: can fly; 0: cannot)
    getBBox(player.pos[0], player.pos[1], player.w, player.h, &player.bbox);

    // exit door
    exit.x = 30; // position in tile
    exit.y = 22; // position in tile
    exit.tiles[0] = 6;
    exit.tiles[1] = 7;
    exit.animation = 0; // 0: first animation; 1: second animation
}

void level2(void)
{
    SMS_loadBGPalette(level2__palette__bin);
    SMS_loadTiles(level2__tiles__bin, BG_TILES, level2__tiles__bin_size);
    SMS_loadTileMap(0, 0, level2__tilemap__bin, level2__tilemap__bin_size);

    // copy level1 to levelCopy
    memcpy(levelCopy, level2_objects, LEVEL_SIZE * sizeof(level2_objects[0]));

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
    monsters[0].w = REXYWIDTH;
    monsters[0].h = REXYHEIGHT;
    getBBox(monsters[0].pos[0], monsters[0].pos[1], monsters[0].w, monsters[0].h, &monsters[0].bbox);

    // powerup array
    getPowerups(powerups);
    powerupCounter = 0; // frame counter for the speed powerup

    // main character
    player.speed = 1; // MUST BE A FACTOR OF 8! (e.g. 1,2,4,8)
    player.sprite = 0;
    player.spedUp = 0; // affected by speed powerup (1: sped up; 0: normal speed)
    player.pos[0] = 8;
    player.pos[1] = 8;
    player.w = PLAYERWIDTH;
    player.h = PLAYERHEIGHT;
    player.canFly = 0; // affected by fly powerup (1: can fly; 0: cannot)
    getBBox(player.pos[0], player.pos[1], player.w, player.h, &player.bbox);

    // exit door
    exit.x = 30; // position in tile
    exit.y = 22; // position in tile
    exit.tiles[0] = 6;
    exit.tiles[1] = 7;
    exit.animation = 0; // 0: first animation; 1: second animation
}
