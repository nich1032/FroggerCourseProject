#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

#include "../include/game.h"
#include "../include/player.h"
#include "../include/crocodile.h"




int main(){

    //creo la pipe

    int pipefd[2];  //il campo 0 è per la lettura, il campo 1 è per la scrittura

    int pid_rana;
    int pid_coccodrilli[MAX_CROCODILES];
    int num_coccodrilli = LANES*2;

    srand(time(NULL));
    //inizializzo schermo
    initscr();

    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);//colore rana
    init_pair(2, COLOR_RED, COLOR_BLACK);//colore coccodrillo

    if (LINES < GAME_HEIGHT || COLS < GAME_WIDTH) {
        endwin();
        fprintf(stderr, "Terminal too small. Needs at least %dx%d\n", GAME_WIDTH, GAME_HEIGHT);
        exit(1);
    }
    resize_term(GAME_HEIGHT, GAME_WIDTH);
    noecho();cbreak();nodelay(stdscr, TRUE);keypad(stdscr, TRUE);

    box(stdscr, ACS_VLINE, ACS_HLINE);

    //pavimento da ristrutturare
    for (int i = 0; i < ACS_VLINE; i++)
    {
        //disegno il pavimento parte inferiore
        mvaddch(FLOOR_HEIGHT, i, ACS_HLINE);

        //disegno il pavimento parte superiore
        mvaddch(3, i, ACS_HLINE);
    }

    
    refresh();

    //creazione dei processi e pipe
    //la rana si muove con le frecce e deve raggiungere il la parte alta dello schermo
    if (pipe(pipefd) == -1){
        perror("Errore nella creazione della pipe!\n");
        _exit(1);
    }

    //avvio processi figli
    if((pid_rana = fork()) == 0){
        close(pipefd[0]);
        rana(pipefd[1]);
    }else if(pid_rana == -1){
        perror("Errore nella creazione del processo rana!\n");
        _exit(1);
    }

    for(int i =0; i<num_coccodrilli; i++){
        if((pid_coccodrilli[i] = fork()) == 0){
            close(pipefd[0]);
            coccodrillo(pipefd[1],i);
        }else if(pid_coccodrilli[i] == -1){
            perror("Errore nella creazione del processo coccodrillo!\n");
            _exit(1);
        }
    }

    close(pipefd[1]);
    game(pipefd[0], num_coccodrilli);
    //chiudo i processi 
    kill(pid_rana, SIGTERM);
    for(int i = 0; i<num_coccodrilli; i++){
        kill(pid_coccodrilli[i], SIGTERM);
    }

        

    endwin();
    return 0;
}