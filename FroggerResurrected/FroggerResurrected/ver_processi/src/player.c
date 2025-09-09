#include "player.h"


/*char rana_sprite[4][3] = {
    {' ', 'O', ' '},
    {'O', 'O', 'O'},
    {' ', 'O', ' '},
    {'O', ' ', 'O'}
};*/

void rana(int pipeout)
{
    struct position p;
    p.c = '$';

    p.x = GAME_WIDTH/2;
    p.y = GAME_HEIGHT-2;
    p.width = 5;
    p.height = 2;

    while (1)
    {
        int ch = getch();

        switch (ch)
        {
        case KEY_UP:
            if(p.y > 1) p.y-=p.height;
            break;
        case KEY_DOWN:
            if(p.y < LINES-2-p.height+1) p.y+=p.height;
            break;
        case KEY_LEFT:
            if(p.x > 1) p.x-=p.width;
            break;
        case KEY_RIGHT:
            if(p.x < COLS-2-p.width+1) p.x+=p.width;
            break;
        }

        //svuoto il buffer di input
        while ((ch = getch()) != ERR);
        
        //scrivo la posizione nella pipe
        write(pipeout, &p, sizeof(struct position));

        //controllo la velocità di movimento
        usleep(200000);
    }
}