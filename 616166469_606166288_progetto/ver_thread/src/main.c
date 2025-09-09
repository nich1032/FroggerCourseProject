#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <ncurses.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>

#include "../include/game.h"
#include "../include/utils.h"

// Stato globale per gestire il segnale
game_state* global_state = NULL;

// Gestore del segnale per aver un'uscita più pulita
void cleanup_handler(int signo) {
    if (global_state) {
        global_state->game_over = true;
    }
}

// Funzione che chiede al giocatore se vuole ricominciare
bool ask_restart() {
    nodelay(stdscr, FALSE); // Abilita l'attesa dell'input
    
    clear();
    mvprintw(LINES/2 - 2, COLS/2 - 20, "Game Over!");
    mvprintw(LINES/2, COLS/2 - 20, "Vuoi ricominciare? (S/N)");
    refresh();
    
    int ch;
    do {
        ch = getch();
        ch = toupper(ch); // Converte in maiuscolo
    } while(ch != 'S' && ch != 'N');
    
    nodelay(stdscr, TRUE); // Ripristina il comportamento non bloccante
    return (ch == 'S');
}

// Una singola partita
void run_game() {
    // Creazione e inizializzazione dello stato di gioco
    game_state state;
    init_game_state(&state);
    global_state = &state;  
    
    // Disegno iniziale dello stato di gioco
    draw_game_state(&state);
    
    // Creazione dei threads
    pthread_t player_tid, game_tid;
    pthread_t crocodile_tids[MAX_CROCODILES];
    crocodile_args* croc_args[MAX_CROCODILES];
    
    // Alloca memoria per gli argomenti prima di creare i thread
    for (int i = 0; i < MAX_CROCODILES; i++) {
        croc_args[i] = malloc(sizeof(crocodile_args));
        if (croc_args[i] == NULL) {
            fprintf(stderr, "Errore di allocazione memoria per i coccodrilli\n");
            // Libera la memoria già allocata
            for (int j = 0; j < i; j++) {
                free(croc_args[j]);
            }
            global_state = NULL;
            return;
        }
        croc_args[i]->state = &state;
        croc_args[i]->id = i;
    }
    
    // Inizio game thread
    pthread_create(&game_tid, NULL, game_thread, &state);
    
    // Inizio player thread
    pthread_create(&player_tid, NULL, player_thread, &state);
    
    // Inizio threads dei coccodrilli
    for (int i = 0; i < MAX_CROCODILES; i++) {
        pthread_create(&crocodile_tids[i], NULL, crocodile_thread, croc_args[i]);
    }
    
    // Chiusura del thread di gioco con join
    pthread_join(game_tid, NULL);
    
    // Game over, clean up
    for (int i = 0; i < MAX_CROCODILES; i++) {
        pthread_cancel(crocodile_tids[i]);
        pthread_join(crocodile_tids[i], NULL);
    }
    pthread_cancel(player_tid);
    pthread_join(player_tid, NULL);
    
    // Clean up proiettili
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (state.bullets[i].pos.active) {
            pthread_cancel(state.bullets[i].thread_id);
            pthread_join(state.bullets[i].thread_id, NULL);
        }
    }
    
    // Libera la memoria degli argomenti dopo la terminazione di tutti i thread
    for (int i = 0; i < MAX_CROCODILES; i++) {
        free(croc_args[i]);
    }
    
    // Pulizia dello stato di gioco
    destroy_game_state(&state);
    global_state = NULL;
    
    // Mostra punteggio finale
    clear();
    mvprintw(LINES/2, COLS/2 - 15, "Game Over - Score: %d", state.score);
    refresh();
    sleep(2);
}

int main(int argc, char* argv[]) {
    // Inizializzazione del seed
    srand(time(NULL));
    
    // Inizializzazione di ncurses
    initscr();
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);  // Colore del player
    init_pair(2, COLOR_RED, COLOR_BLACK);    // Colore del coccodrillo
    init_pair(3, COLOR_BLUE, COLOR_CYAN);    // bordi del fiume
    init_pair(5, COLOR_YELLOW, COLOR_RED);   // Colore della terra
    init_pair(6, COLOR_GREEN, COLOR_BLACK);  // tana vuota
    init_pair(7, COLOR_YELLOW, COLOR_GREEN); // tana occupata
    
    // Controllo dimensione del terminale
    if (LINES < GAME_HEIGHT || COLS < GAME_WIDTH) {
        endwin();
        fprintf(stderr, "Terminal too small. Needs at least %dx%d but got %dx%d\n", 
                GAME_WIDTH, GAME_HEIGHT, COLS, LINES);
        exit(1);
    }
    
    // Setta il terminale
    resize_term(GAME_HEIGHT, GAME_WIDTH);
    noecho();
    cbreak();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    curs_set(0);  
    
    // Pulizia iniziale dello schermo
    clear();
    box(stdscr, ACS_VLINE, ACS_HLINE);
    refresh();
    
    // Segnali di terminazione
    signal(SIGINT, cleanup_handler);
    signal(SIGTERM, cleanup_handler);
    
    // Esegui una singola partita
    run_game();
    
    // Chiedi se vuole ricominciare
    bool restart = ask_restart();
    
    // Chiusura di ncurses
    endwin();
    
    if (restart) {
        // Riavvia il processo stesso
        char* program_path = argv[0];
        execl(program_path, program_path, NULL);
        // Se execl fallisce:
        perror("Failed to restart game");
        exit(1);
    }
    
    return 0;
}
