#ifndef PROJECTILES_H
#define PROJECTILES_H

#include "game.h"
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

/*
    * Funzione che gestisce il processo proiettile 
    * @param pipeout pipe per la scrittura della posizione
    * @param p posizione iniziale del proiettile
    * @param direction direzione del proiettile (1 = destra, -1 = sinistra)
*/
void bullet(int pipeout, struct position *p, int direction);

#endif // PROJECTILES_H
