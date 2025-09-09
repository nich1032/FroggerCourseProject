#ifndef UTILS_H
#define UTILS_H

#include "game.h"
#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <time.h>

// Drawing functions
void draw_time_bar(int remaining_time, int max_time);
void draw_score(int score);
void draw_river_borders(void);
void clear_frog_position(struct position *pos);
void draw_game_borders(void);
void init_dens(struct tana tane[]);
void draw_dens(struct tana tane[]);
bool check_den_collision(struct position *rana_pos, struct tana tane[]);
void clear_entities(struct position *rana_pos, struct position crocodile_positions[], 
                   int num_coccodrilli, struct position bullets[], int max_bullets);
void draw_frog(struct position *rana_pos);
void draw_crocodiles(struct position crocodile_positions[], int num_coccodrilli);
void draw_bullets(struct position bullets[]);
bool is_invalid_top_area(struct position *rana_pos, struct tana tane[]);

#endif // UTILS_H
