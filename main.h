#ifndef MAIN_H
#define MAIN_H

#define BG_TILES 0
#define SPRITE_TILES 256
#define LEVEL_SIZE 768 // in tiles
#define PLAYER_TILES SPRITE_TILES
#define EXIT_TILES (SPRITE_TILES + 2)
#define REXY_TILES_LEFT (SPRITE_TILES + 4)
#define REXY_TILES_RIGHT (SPRITE_TILES + 6)
#define POCCO_TILES (SPRITE_TILES + 8)
#define MINPLAYERY 8
#define MAXPLAYERY (192 - 16)
#define MINPLAYERX 8
#define MAXPLAYERX (256 - 16)
#define PLAYERHEIGHT 8
#define PLAYERWIDTH 8
#define MAXSTEPS 8
#define MAX_SPEED_POWERUP 800   // duration of speed powerup in frames
#define TILE_ANIMATION_FRAME 16 // number of frames before changing tile for animation
#define MAX_MONSTERS 10         // allowed maximum number of monsters per level

unsigned int ks;
unsigned char exit;        // 0: exit was reached; 1: exit was not reached yet
unsigned char exitPosPixel[2]; // exit position in px
unsigned char exitPosTile[2];  // exit position in tiles
unsigned char step;            // 0 to 7 (counts how many steps the player has taken within a tile)
unsigned int speedPowerupCounter;
unsigned char speedPowerupPos[2];
unsigned char speedPowerupTiles[2];  // speed power up tile index in tileset
unsigned char speedPowerupAnimation; // 0: first animation; 1: second animation
unsigned char frameCounter;          // keep track of frame number (e.g. for animation)
enum Direction
{
    UNKNOWN,
    UP,
    DOWN,
    LEFT,
    RIGHT
};
// list of level objects and their codes (as found in level*.csv in assets folder)
enum LevelObject
{
    WALL = 0x00,
    TERRAIN = 0x01,
    POWERUP_SPEED = 0x02,
    POWERUP_FLY = 0x03,
    POWERUP_REVERSE = 0x04
};
// list of enemy names
enum MonsterName
{
    REXY,
    POCCO
};
// generic struct for enemies
struct Monster
{
    enum MonsterName name;
    enum Direction dir;
    unsigned char speed;     // MUST BE A FACTOR OF 8! (e.g. 1,2,4,8)
    unsigned char pos[2];    // x and y coordinates (in px)
    unsigned char limits[4]; // [xmin, xmax, ymin, ymax] movement limits (in px)
    unsigned char sprite;    // used to animate the monster between two sprites (either 0 or 1)
};
struct Monster monsters[MAX_MONSTERS]; // global array that will store the monsters for a level
unsigned char activeMonsters;          // keep track of the number of monsters for the current level
// player struct
struct Player
{
    enum Direction dir;
    unsigned char speed;     // MUST BE A FACTOR OF 8! (e.g. 1,2,4,8)
    unsigned char pos[2];    // x and y coordinates (in px)
    unsigned char sprite;    // used to animate the monster between two sprites (either 0 or 1)
    unsigned char isMoving;  // 0: not-moving; 1: moving
    unsigned char step;      // 0 to 7 (counts how many steps the player has taken within a tile)
    unsigned char spedUp;    // affected by speed powerup (1: sped up; 0: normal speed)
};
struct Player player;

enum Direction dir; // check player next direction for collision detection

// store level copies to allow for changes (since banks store them as const)
// this is needed for (e.g.) overwrite a powerup tile with a terrain tile after the powerup is taken
unsigned char levelCopy[LEVEL_SIZE];

// DRAW ROUTINES
void drawPlayer(void);
void drawExit(void);
void drawRexy(struct Monster *rexy);
void drawPocco(struct Monster *pocco);
void drawMonster(struct Monster *monster);

// MOVE ROUTINES
void movePlayer(enum Direction dir);
void moveRexy(struct Monster *rexy);
void movePocco(struct Monster *pocco);
void moveMonster(struct Monster *monster);

// COLLISION ROUTINES
unsigned char monsterCollision(void);
enum LevelObject levelCollision(enum Direction dir);
enum Direction getDirection(void);

#endif