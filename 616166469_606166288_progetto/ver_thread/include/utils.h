#ifndef UTILS_H
#define UTILS_H

#include "game.h"

// Funzioni di disegno
void draw_time_bar(int remaining_time, int max_time);
void draw_score(int score);
void draw_river_borders(void);
void clear_frog_position(position *pos);
void draw_game_borders(void);
void init_dens(tana tane[]);
void draw_dens(tana tane[]);
void safe_mvaddch(int y, int x, chtype ch, pthread_mutex_t* screen_mutex);
void draw_game_state(game_state* state);

#endif // UTILS_H