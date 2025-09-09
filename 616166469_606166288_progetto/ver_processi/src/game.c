#include "../include/game.h"
#include "../include/utils.h"
#include "../include/audio.h"
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

// Librerie usate solo per audio
#include <sys/ipc.h> 
#include <sys/shm.h> 

// Funzione per gestire la fine del gioco (game over, vittoria, tempo scaduto)
bool handle_game_end(int *vite, int score, const char *message) {
    clear();
    mvprintw(LINES / 2, COLS / 2 - 10, "%s", message);
    mvprintw((LINES/2) + 1, COLS/2-15, "SCORE FINALE: %d", score);
    mvprintw((LINES/2) + 3, COLS/2-15, "Vuoi giocare ancora? (s/n)");
    refresh();
    
    // Cambia modalità input per leggere correttamente la risposta dell'utente
    nocbreak(); // Disattiva la modalità cbreak
    cbreak(); // La riattivo per assicurarmi che sia in questa modalità
    nodelay(stdscr, FALSE); // Modalità bloccante
    flushinp(); // Pulisce il buffer di input
    
    // Aspetta la risposta dell'utente
    int risposta;
    do {
        risposta = getch();
    } while (risposta != 's' && risposta != 'S' && risposta != 'n' && risposta != 'N');
    
    // Ripristina la modalità di input per il gioco
    nodelay(stdscr, TRUE); // Ritorna alla modalità non bloccante
    
    if (risposta == 's' || risposta == 'S') {
        *vite = 3; // Resetta le vite per la nuova partita
        return true; // Segnala che il giocatore vuole giocare ancora
    } else {
        return false; // Segnala che il giocatore vuole uscire
    }
}

void destroy_shared_memory(int shmid, void *shmaddr) {
    if (shmdt(shmaddr) == -1) {
        perror("shmdt");
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl IPC_RMID");
    }
    cleanup_audio(); // Added to terminate audio processes
}

bool game(int pipein,int pipeToFrog,int num_coccodrilli,int *vite,int pausepipe)
{
    // Avvia la musica di sottofondo
    toggle_background_music(true);

    struct position p;
    srand(time(NULL));
    //la rana inizia dal centro dello schermo
    struct position rana_pos = {
        .c = '$', 
        .x = GAME_WIDTH/2, // Inizia al centro
        .y = GAME_HEIGHT-2, 
        .width = 2, 
        .height = 5,
        .id = 0
    };
    struct position crocodile_positions [num_coccodrilli];
    struct position bullets[MAX_BULLETS];
    struct tana tane[NUM_TANE];
    init_dens(tane);
    int tane_occupate = 0;
    int max_height_reached = GAME_HEIGHT-2; // Initialize to starting position

    bool game_over = false;
    
    // Imposta il file descriptor pausepipe come non bloccante
    fcntl(pausepipe, F_SETFL, O_NONBLOCK);
    bool pause = false;

    //Inzializzazione delle variabili per la barra del tempo
    int max_time = 30; //ho messo 30 secondi, se necessario si può incrementare
    int remaining_time = max_time;
    time_t last_update = time(NULL);
    draw_time_bar(remaining_time, max_time);

    //Inizializzazione dello score
    int score=0;
    draw_score(score);

    // Initialize all crocodile positions
    // i coccodrilli si dividono in corsie
    for (int i = 0; i < num_coccodrilli; i++) {
            crocodile_positions[i] = (struct position) {
            .c = 'C',
            .height = 2,  // Tiene la stessa altezza della rana
            .id = i
        };
    }

    //inizializzo i proiettili
    for (int i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active = 0;
        bullets[i].id = i;
        bullets[i].collision = 0;
    }
    
    while (!game_over && *vite>0)
    {
        time_t current_time = time(NULL);


        // Controlla se è arrivato un comando di pausa
        char pause_cmd;
        if (read(pausepipe, &pause_cmd, sizeof(char)) > 0) {
            if (pause_cmd == 'p') {
                pause = !pause;
                
                if (pause) {
                    // Mostra messaggio di pausa
                    mvprintw(LINES/2, COLS/2-10, "GIOCO IN PAUSA");
                    mvprintw(LINES/2+1, COLS/2-15, "Premi 'p' per continuare");
                    refresh();

                    // Invia SIGSTOP a tutti i processi coccodrillo
                    for (int i = 0; i < num_coccodrilli; i++) {
                        kill(crocodile_positions[i].pid, SIGSTOP);
                    }
                     // Invia SIGSTOP a tutti i processi proiettile attivi
                    for (int i = 0; i < MAX_BULLETS; i++) {
                        if (bullets[i].active && bullets[i].pid > 0) {
                            // Verifica che il processo sia ancora vivo
                            if (kill(bullets[i].pid, 0) != -1) {
                                kill(bullets[i].pid, SIGSTOP);
                            }
                        }
                    }
                } else {
                    // Aggiorna il tempo per evitare salti
                    last_update = time(NULL);

                    // Cancella messaggio di pausa
                    mvprintw(LINES/2, COLS/2-10, "                 ");
                    mvprintw(LINES/2+1, COLS/2-15, "                        ");
                    refresh();

                    // Invia SIGCONT a tutti i processi coccodrillo
                    for (int i = 0; i < num_coccodrilli; i++) {
                        kill(crocodile_positions[i].pid, SIGCONT);
                    }

                    // Invia SIGCONT a tutti i processi proiettile attivi
                    for (int i = 0; i < MAX_BULLETS; i++) {
                        if (bullets[i].active && bullets[i].pid > 0) {
                            // Verifica che il processo sia ancora vivo
                            if (kill(bullets[i].pid, 0) != -1) {
                                kill(bullets[i].pid, SIGCONT);
                            }
                        }
                    }

                    // Svuota le pipe per evitare l'accumulo di posizioni obsolete
                    char buffer[1024];
                    fcntl(pipein, F_SETFL, O_NONBLOCK);
                    while (read(pipein, buffer, sizeof(buffer)) > 0) {
                        // Svuota il buffer
                    }
                    fcntl(pipein, F_SETFL, 0); // Ripristina modalità bloccante
                    
                }
            }
        }

        // Se il gioco è in pausa, non fare nulla e controlla solo per comandi di pausa
        if (pause) {
            // Non leggere dalle altre pipe
            // Non aggiornare la posizione
            // Non fare collision detection
            usleep(50000);  // Attesa breve per non consumare troppa CPU
            continue;
        }
      
        // Leggi la posizione dalla pipe
        ssize_t r = read(pipein, &p, sizeof(struct position));
        if (r <= 0) {
            mvprintw(LINES/2, COLS/2-10, "Pipe read error: %s", strerror(errno));
            refresh();
            sleep(1);
            continue;
        }

        //disegno lo score
        draw_score(score);
        //disegno il numero di vite rimanenti
        mvprintw(LINES-1,GAME_WIDTH-20,"Vite: %d",*vite); //ho riadattato a GAME_WIDTH-20 per avere sia vite che score a schermo

        clear_entities(&rana_pos, crocodile_positions, num_coccodrilli, bullets, MAX_BULLETS);

        //aggiorno la posizione in base al carattere letto
        if (p.c == '$') {
            int crocodile_direction = 0;
            bool was_on_crocodile = rana_coccodrillo(&rana_pos, crocodile_positions, num_coccodrilli, &crocodile_direction);
            if (was_on_crocodile) {
                //Aggiorna score
                // Aggiorna la posizione della rana in base all'input del giocatore
                rana_pos.y = p.y;
                rana_pos.x = p.x;
                if (rana_pos.y < max_height_reached) {
                    score += 5;
                    max_height_reached = rana_pos.y;
                }
                // Aggiungi anche il movimento del coccodrillo
                rana_pos.x -= crocodile_direction;
                fcntl(pipeToFrog, F_SETFL, O_NONBLOCK);
                write(pipeToFrog, &rana_pos, sizeof(struct position));
                

            } else {
                //se la rana non è su un coccodrillo, controllo se e caduta in acqua
                if(frog_on_the_water(&rana_pos)){
                     //resetta lo score e riduce di 1 le vite
                    play_sound(SOUND_SPLASH); // Frog fell in water
                    score=0;
                    (*vite)--;
                    max_height_reached = GAME_HEIGHT-2; // Resetta l'altezza MAX

                    if (*vite > 0)
                    {
                        //stampa messaggio RAANA IN ACQUA al centro dello schermo
                        mvprintw(LINES/2, COLS/2-10, "RANA IN ACQUA!");
                        score=0;
                        refresh();
                        // resetta la posizione della rana
                        rana_pos.x = GAME_WIDTH/2;
                        rana_pos.y = GAME_HEIGHT-2;
                        write(pipeToFrog, &rana_pos, sizeof(struct position));
                        //reset del timer
                        remaining_time = max_time;
                        napms(2000);

                        continue;
                    }
                    else{
                        return handle_game_end(vite, score, "GAME OVER!");
                    }
                }else{
                    rana_pos = p; // Aggiorna la posizione se non si è su un coccodrillo
                }
            }
            // Controlla che la rana non esca dallo schermo
            if (rana_pos.x < 1) rana_pos.x = 1;
            if (rana_pos.x > GAME_WIDTH - rana_pos.width - 1) 
                rana_pos.x = GAME_WIDTH - rana_pos.width - 1;
        } else if (p.c == 'C') { // Aggiorna la posizione del coccodrillo
            for (int i = 0; i < num_coccodrilli; i++) {
                if (crocodile_positions[i].id == p.id) {
                    crocodile_positions[i] = p;
                    break;
                }
            }
        }else if(p.c == '@' || p.c == '*'){
            int found = 0;
            
            // Controllo se il processo è ancora attivo
            // Se non è attivo, lo segno come inattivo
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (bullets[i].active) {
                    if (kill(bullets[i].pid, 0) == -1 && errno == ESRCH) {
                        bullets[i].active = 0;
                    }
                }
            }
            
            // Processo proiettile attivo
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (bullets[i].active && bullets[i].pid == p.pid) {
                    int was_collided = bullets[i].collision;
                    bullets[i].x = p.x;
                    bullets[i].y = p.y;
                    bullets[i].active = p.active; 
                    bullets[i].collision = was_collided || p.collision;
                    found = 1;
                    break;
                }
            }
            
            // Se non è stato trovato e il proiettile è attivo, lo aggiungo
            if (!found && p.active) {
                for (int i = 0; i < MAX_BULLETS; i++) {
                    if (!bullets[i].active) {
                        bullets[i] = p;
                        break;
                    }
                }
            }
        }

        //controllo la condizione di vittoria, la rana ha raggiunto la fine dello schermo
        if (rana_pos.y <= 1) { 
            if (is_invalid_top_area(&rana_pos, tane)) {
                // La rana prova ad entrare in una tana occupata o zona off limits
                play_sound(SOUND_SPLASH); // Frog hit invalid area
                (*vite)--;
                max_height_reached = GAME_HEIGHT-2; // Reset max height
                
                if (*vite > 0) {
                    // Display message
                    mvprintw(LINES/2, COLS/2-15, "AREA NON DISPONIBILE!");
                    refresh();
                    
                    // Resetta posizione della rana
                    rana_pos.x = GAME_WIDTH/2;
                    rana_pos.y = GAME_HEIGHT-2;
                    write(pipeToFrog, &rana_pos, sizeof(struct position));
                    
                    // Resetta il timer
                    remaining_time = max_time;
                    napms(2000);
                    
                    continue;
                } else {
                    return handle_game_end(vite, score, "GAME OVER!");
                }
            }
            else if(check_den_collision(&rana_pos, tane)) {
                // Riproduci il suono della tana conquistata
                play_sound(SOUND_TANA_ENTER);
                
                score+=100;
                tane_occupate++;
                //reset del timer
                remaining_time = max_time;
                max_height_reached = GAME_HEIGHT-2; // Reset max height

                // Reset rana position
                rana_pos.x = GAME_WIDTH/2;
                rana_pos.y = GAME_HEIGHT-2;
                write(pipeToFrog, &rana_pos, sizeof(struct position));
                
                if(tane_occupate == NUM_TANE) {
                    return handle_game_end(vite, score, "HAI VINTO!");
                }
            }
        }
        

       //collisione proiettili
       //controllo se la rana è stata colpita
        for(int i=0;i<MAX_BULLETS;i++){
            if(bullets[i].c=='@'&&bullets[i].x==rana_pos.x&&bullets[i].y==rana_pos.y&& !bullets[i].collision){
                play_sound(SOUND_SPLASH); // Frog hit by projectile
                score=0;
                (*vite)--;
                if(*vite > 0) {
                    clear_frog_position(&rana_pos);
                    
                    rana_pos.x = GAME_WIDTH/2;
                    rana_pos.y = GAME_HEIGHT-2;
                    
                    ssize_t w = write(pipeToFrog, &rana_pos, sizeof(struct position));
                    if(w < 0) {
                        mvprintw(0, 0, "Pipe write error: %s", strerror(errno));
                        refresh();
                    }

                    // Si resetta lo score
                    score=0;
                    
                    mvprintw(LINES/2, COLS/2-10, "RANA COLPITA! Vite: %d", *vite);
                    refresh();
                    napms(2000);
                    
                    // Reset timer
                    remaining_time = max_time;
                    
                    // Riprisitina la collsione del proiettile
                    bullets[i].collision = 1;
                    
                    attron(COLOR_PAIR(1));
                    for (int i = 0; i < rana_pos.height; i++)
                    {
                        for (int j = 0; j < rana_pos.width; j++)
                        {
                            mvaddch(rana_pos.y + i, rana_pos.x + j, rana_sprite[i][j]);
                        }
                    }
                    attroff(COLOR_PAIR(1));
                    refresh();
                    
                    break; // Esce dal loop di controllo del proiettile
                } else {
                    return handle_game_end(vite, score, "GAME OVER!");
                }
            }
        }
       //collisione due proiettili
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].active && bullets[i].c == '@' && !bullets[i].collision) {
                for (int j = 0; j < MAX_BULLETS; j++) {
                    if (bullets[j].active && bullets[j].c == '*'&& !bullets[j].collision) {
                        if (bullets[i].x == bullets[j].x && bullets[i].y == bullets[j].y && !bullets[j].collision) {

                            score+=50;
                          
                            bullets[i].collision = 1;
                            bullets[j].collision = 1;

                            mvaddch(bullets[i].y, bullets[i].x, 'X');
                            mvaddch(bullets[j].y, bullets[j].x, 'X');
                        }
                    }
                }
            }
        }

        // Se è passato un secondo, aggiorna il tempo rimanente

        if (current_time - last_update >= 1) {

        //aggiorna le variabili che gestiscono il tempo
            last_update = current_time;
            remaining_time--;

            // Disegna la barra del tempo
            draw_time_bar(remaining_time, max_time);

            // Controlla se il tempo è finito
            if (remaining_time <= 0) {
                return handle_game_end(vite, score, "TEMPO SCADUTO!");
            }
        }

        // Disegna i bordi dopo aver ripulito lo schermo
        draw_river_borders();
        draw_game_borders();
        draw_dens(tane);

        //disegno i coccodrilli
        draw_crocodiles(crocodile_positions, num_coccodrilli);
        

        //disegno i proiettili
        draw_bullets(bullets);
        //disegno la rana

        draw_frog(&rana_pos);
        refresh();
    }
    
    // Se il gioco termina normalmente (non vittoria o game over), 
    // restituisci false per indicare che si vuole uscire dal gioco
    return false;
}

bool rana_coccodrillo(struct position *rana_pos, struct position crocodile_positions[], int num_coccodrilli, int *direction) {
    for (int i = 0; i < num_coccodrilli; i++) {
        // Controllo se la rana è su un coccodrillo
        if (rana_pos->y == crocodile_positions[i].y && 
            rana_pos->x >= crocodile_positions[i].x && 
            rana_pos->x <= crocodile_positions[i].x + crocodile_positions[i].width - rana_pos->width) {
            
            // Setta la direzione in base alla lane del coccodrillo
            int lane = (crocodile_positions[i].id/2) % LANES;
            *direction = (lane % 2 == 0) ? -1 : 1;
            return true; // La rana è su un coccdrillo
        }
    }
    return false;
}

bool frog_on_the_water(struct position *rana_pos){
    if (rana_pos->y < FLOOR_HEIGHT && rana_pos->y > 3){
        return true;
    }
    return false;
}
