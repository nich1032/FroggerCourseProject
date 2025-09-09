#ifndef GAME_H
#define GAME_H

#include <unistd.h>
#include <sys/types.h>
#include <ncurses.h>    
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#define GAME_WIDTH 80    // Standard terminal width
#define GAME_HEIGHT 24   // Standard terminal height
#define FLOOR_HEIGHT 20  // Actual playable area height
#define LANES 8         // Number of lanes for obstacles
#define LANE_HEIGHT 2   // Height of each lane
#define MAX_CROCODILES 8 // Maximum number of crocodiles


struct position
{
    char c; // $ per la rana, C per il coccodrillo
    int x;
    int y;
    int width;
    int height;
    int id;
};

void game(int,int);

bool rana_coccodrillo(struct position *rana_pos, struct position crocodile_positions[], int num_coccodrilli, int *direction);
#endif // GAME_H