#include "../include/game.h"

// Definizione sprite della rana
char rana_sprite[2][5] = {
    {' ', ' ', 'O', ' ', ' '},
    {'_', '`', 'O', '\'', '_'}
};

//Sprite per il movimento del coccodrillo verso sinistra o destra
char crocodile_sprite_sx[2][15] = {
    {' ', '_', '_', '_', '/', '^', '\\', '_', '_', '_', '_', '_', '_', '_', '_'},
    {'/', '=', '=', '=', '=', '=', '=', '=', '=', '=', '=', '=', '=', '=', '='}
};

char crocodile_sprite_dx[2][15] = {
    {'_', '_', '_', '_', '_', '_', '_', '/', '^', '\\', '_', '_', '_', '_', ' '},
    {'=', '=', '=', '=', '=', '=', '=', '=', '=', '=', '=', '=', '=', '=', '\\'}
};
