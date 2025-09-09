#ifndef GAME_H
#define GAME_H


#include <unistd.h>
#include <sys/types.h>
#include <ncurses.h>    
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <signal.h>



#define GAME_WIDTH 80    // Standard terminal width
#define GAME_HEIGHT 24   // Standard terminal height
#define FLOOR_HEIGHT 20  // Actual playable area height
#define LANES 8         // Number of lanes for obstacles
#define LANE_HEIGHT 2   // Height of each lane
#define MAX_CROCODILES 16 // Maximum number of crocodiles
#define MAX_BULLETS 100  // Maximum number of bullets

#define NUM_TANE 5
#define TANA_WIDTH 7
#define TANA_HEIGHT 1

extern char rana_sprite[2][5];
extern char crocodile_sprite_sx[2][15];
extern char crocodile_sprite_dx[2][15];


struct position
{
    char c; // $ per la rana, C per il coccodrillo
    int x;
    int y;
    int width;
    int height;
    int id;
    int active;
    int collision;
    pid_t pid;
};

struct tana {
    int x;
    int y;
    bool occupata;
};

bool game(int,int,int,int*,int);

bool rana_coccodrillo(struct position *rana_pos, struct position crocodile_positions[], int num_coccodrilli, int *direction);

bool frog_on_the_water(struct position *rana_pos);


#endif // GAME_H