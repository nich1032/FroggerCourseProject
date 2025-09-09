#include "../include/player.h"
#include "../include/projectiles.h"
#include "../include/audio.h"
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

void rana(int pipeout,int pipein,int pausepipe)
{
    struct position p;
    p.c = '$';

    p.x = GAME_WIDTH/2;
    p.y = GAME_HEIGHT-2;
    p.width = 5;
    p.height = 2;

    fcntl(pipein, F_SETFL, O_NONBLOCK);

    while (1)
    {

        // Controlla se ci sono dati da leggere dalla pipe
        struct position update;
        if (read(pipein, &update, sizeof(struct position)) > 0) {
            // Aggiorna la posizione con quella ricevuta dal game
            p.x = update.x;
            p.y = update.y;
        }

        int ch = getch();
        
        if (ch == 'p' || ch == 'P') {
            // Invia il comando di pausa (1 byte è sufficiente)
            char pause_cmd = 'p';
            write(pausepipe, &pause_cmd, sizeof(char));
            continue;  // Salta il resto del ciclo
        }
        
        switch (ch)
        {
        case KEY_UP:
            if(p.y > 1) p.y-=p.height;
            break;
        case KEY_DOWN:
            if(p.y < LINES-2-p.height+1) p.y+=p.height;
            break;
        case KEY_LEFT:
            if(p.x > 1) p.x-=2;
            break;
        case KEY_RIGHT:
            if(p.x < COLS-2-p.width+1) p.x+=2;
            break;

        case ' ':
            struct position right_bullet = p;
            right_bullet.c = '*';
            right_bullet.x = p.x + p.width; // Parte dal bordo destro della rana
            
            // Crea proiettile sinistro
            struct position left_bullet = p;
            left_bullet.c = '*';
            left_bullet.x = p.x - 1; // Parte dal bordo sinistro della rana

            // Riproduci il suono dello sparo
            play_sound(SOUND_SHOOT);
            
            // Lancia il proiettile destro
            if(fork() == 0) {
                bullet(pipeout, &right_bullet, 1);
                _exit(0);
            }
            
            // Lancia il proiettile sinsitro 
            if(fork() == 0) {
                bullet(pipeout, &left_bullet, -1);
                _exit(0);
            }
            break;

            case 'q':
                //teletrasporto il player poco prima della tana
                p.x = GAME_WIDTH/2;
                p.y = 2;
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
