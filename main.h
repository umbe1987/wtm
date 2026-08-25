#ifndef MAIN_H
#define MAIN_H

#define BG_TILES 0
#define SPRITE_TILES 256
#define LEVEL_SIZE 768 // in tiles
#define PLAYER_TILES SPRITE_TILES
#define EXIT_TILES (SPRITE_TILES + 2)
#define FLY_TILES (SPRITE_TILES + 3)
#define REXY_TILES_LEFT (SPRITE_TILES + 4)
#define REXY_TILES_RIGHT (SPRITE_TILES + 6)
#define POCCO_TILES (SPRITE_TILES + 8)
#define MINPLAYERY 8
#define MAXPLAYERY (192 - 16)
#define MINPLAYERX 8
#define MAXPLAYERX (256 - 16)
#define PLAYERHEIGHT 8
#define PLAYERWIDTH 8
#define REXYHEIGHT 8
#define REXYWIDTH 8
#define POCCOHEIGHT 8
#define POCCOWIDTH 16
#define MAXSTEPS 8
#define MAX_POWERUP_COUNTER 800 // duration of powerup in frames
#define TILE_ANIMATION_FRAME 16 // number of frames before changing tile for animation
#define MAX_MONSTERS 10         // allowed maximum number of monsters per level
#define MAX_POWERUPS 10         // allowed maximum number of powerups per level

unsigned int ks;
unsigned char end;             // 0: exit was reached; 1: exit was not reached yet
unsigned char step;            // 0 to 7 (counts how many steps the player has taken within a tile)
unsigned int powerupCounter;   // TODO: for simplicity, we keep only one common counter for all powerups.
                               // In the future, each powerup will have its own counter
unsigned char frameCounter;    // keep track of frame number (e.g. for animation)
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
    EXIT = 0x02,
    POWERUP_FLY = 0x03,
    POWERUP_SPEED = 0x04,
    POWERUP_REVERSE = 0x05
};
// struct for bounding-box
struct BoundingBox
{
    unsigned char xmin;
    unsigned char xmax;
    unsigned char ymin;
    unsigned char ymax;
};
void getBBox(unsigned char x, unsigned char y,
             unsigned char w, unsigned char h,
             struct BoundingBox *bbox);
// struct for power-ups
struct PowerUp
{
    enum LevelObject type;   // type of power up
    unsigned int ix;         // power up level index
    unsigned char x;         // x coord of powerup (in tile)
    unsigned char y;         // y coord of powerup (in tile)
    unsigned char counter;   // powerup frame counter (max is MAX_POWERUP_COUNTER)
    unsigned char tiles[2];  // index of powerup in the tileset
    unsigned char animation; // 0: first animation; 1: second animation
    unsigned char taken;     // 0: not taken; 1: taken (to remove it from the level)
};
struct PowerUp powerups[MAX_POWERUPS]; // global array that will store the powerups for a level
unsigned char activePowerUps;          // keep track of the number of powerups for the current level
// struct for exit
struct Exit
{
    unsigned char x;         // x coord of powerup (in tile)
    unsigned char y;         // y coord of powerup (in tile)
    unsigned char tiles[2];  // index of powerup in the tileset
    unsigned char animation; // 0: first animation; 1: second animation
};
struct Exit exit;
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
    unsigned char pos[2];    // top-left x and y map coordinates of this sprite (in px)
    unsigned char w;         // width of the sprite (in px)
    unsigned char h;         // height of the sprite (in px)
    unsigned char limits[4]; // [xmin, xmax, ymin, ymax] movement limits (in px)
    unsigned char sprite;    // used to animate the monster between two sprites (either 0 or 1)
    struct BoundingBox bbox; // bounding-box for collision detection
};
struct Monster monsters[MAX_MONSTERS]; // global array that will store the monsters for a level
unsigned char activeMonsters;          // keep track of the number of monsters for the current level
// player struct
struct Player
{
    enum Direction dir;
    unsigned char speed;     // MUST BE A FACTOR OF 8! (e.g. 1,2,4,8)
    unsigned char pos[2];    // top-left x and y map coordinates of this sprite (in px)
    unsigned char w;         // with of the sprite (in px)
    unsigned char h;         // height of the sprite (in px)
    unsigned char sprite;    // used to animate the monster between two sprites (either 0 or 1)
    unsigned char isMoving;  // 0: not-moving; 1: moving
    unsigned char step;      // 0 to 7 (counts how many steps the player has taken within a tile)
    unsigned char spedUp;    // affected by speed powerup (1: sped up; 0: normal speed)
    unsigned char canFly;    // affected by fly powerup (1: can fly; 0: cannot)
    struct BoundingBox bbox; // bounding-box for collision detection
};
struct Player player;

enum Direction dir; // check player next direction for collision detection

// store level copies to allow for changes (since banks store them as const)
// this is needed for (e.g.) overwrite a powerup tile with a terrain tile after the powerup is taken
enum LevelObject levelCopy[LEVEL_SIZE];

// DRAW ROUTINES
void drawPlayer(void);
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
unsigned int nextMove(enum Direction dir);
enum Direction getDirection(void);

#endif