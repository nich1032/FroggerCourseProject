#ifndef CROCODILE_H
#define CROCODILE_H

#include "../include/game.h"
#include "../include/projectiles.h"

void coccodrillo(int pipeout, int id);
void handle_border_collision(struct position *p, int *original_width);
void update_position(struct position *p, int direction);

#endif // CROCODILE_H