#include "../include/game.h"
#include <unistd.h>
#include <time.h>
#include <stdlib.h>


void* crocodile_thread(void* arg) {
    crocodile_args* args = (crocodile_args*)arg;
    game_state* state = args->state;
    int id = args->id;
    
    // Copia posizione originale per il calcolo della velocità
    pthread_mutex_lock(&state->crocodiles[id].mutex);
    int lane = (state->crocodiles[id].id / 2) % LANES;
    int direction = (lane % 2 == 0) ? 1 : -1;
    int original_width = state->crocodiles[id].width;
    pthread_mutex_unlock(&state->crocodiles[id].mutex);
    
    // Seed random number generator uniquely for this thread
    srand(time(NULL) ^ id);
    
    // Calcolo della velocità di movimento in base alla corsia (valori bassi=velocità di movimento maggiore)
    int speed;
    switch(lane % 4) {
        case 0:
            speed = 250000; // corsia 0, 4, 8 - media
            break;
        case 1:
            speed = 300000; // corsia 1, 5, 9 - lenta
            break;
        case 2:
            speed = 180000; // corsia 2, 6, 10 - veloce
            break;
        case 3:
            speed = 220000; // corsia 3, 7, 11 - medio-veloce
            break;
        default:
            speed = 250000; // fallback
    }
    
    // Aggiunta randomica alla velocità di movimento
    speed = speed * (90 + rand() % 21) / 100;
    
    while (!state->game_over) {
        // Controllo per la pausa
        if (state->game_paused) {
            usleep(100000); 
            continue;
        }
        
        // Blocca la posizione per permetterne l'aggiornamento
        pthread_mutex_lock(&state->crocodiles[id].mutex);
        
        // Controlla se il player è su questo coccodrillo per garantire
        // movimento sincronizzato
        bool has_player = (state->player_on_crocodile && state->player_crocodile_id == id);
        
        // Aggiorna la posizione
        state->crocodiles[id].x += direction;
        
        // Gestione dei confini
        if (direction > 0) {  // Ci si sposta a destra
            if (state->crocodiles[id].x + state->crocodiles[id].width > GAME_WIDTH - 1) {
                // Uscita totale o parziale verso destra
                int overflow = (state->crocodiles[id].x + state->crocodiles[id].width) - (GAME_WIDTH - 1);
                state->crocodiles[id].width = state->crocodiles[id].width - overflow;
                
                if (state->crocodiles[id].width <= 0) {
                    // Wrap around to left side
                    state->crocodiles[id].x = 1;
                    state->crocodiles[id].width = 1; // Inizia a crearsi da sinistra
                    
                    // Se il player è sul coccodrillo che sta sparendo, cade in acqua
                    if (has_player) {
                        pthread_mutex_lock(&state->game_mutex);
                        state->player_on_crocodile = false;
                        state->player_crocodile_id = -1;
                        pthread_mutex_unlock(&state->game_mutex);
                    }
                }
            } else if (state->crocodiles[id].width < original_width) {
                // Continua la creazione mentre si muove da sinistra
                state->crocodiles[id].width++;
            }
        } else {  // Spostamento verso destra
            if (state->crocodiles[id].x <= 0) {
                // Uscita totale o parziale verso sinistra
                state->crocodiles[id].width = state->crocodiles[id].width - 1;
                state->crocodiles[id].x = 1;
                
                if (state->crocodiles[id].width <= 0) {
                    // Wrap around to right side
                    state->crocodiles[id].x = GAME_WIDTH - 2;
                    state->crocodiles[id].width = 1; // Inizia a crearsi da destra
                    
                    // Se il player è sul coccodrillo che sta sparendo, cade in acqua
                    if (has_player) {
                        pthread_mutex_lock(&state->game_mutex);
                        state->player_on_crocodile = false;
                        state->player_crocodile_id = -1;
                        pthread_mutex_unlock(&state->game_mutex);
                    }
                }
            } else if (state->crocodiles[id].width < original_width) {
                // Continua la creazione mentre si muove da destra
                state->crocodiles[id].width++;
                state->crocodiles[id].x--;
                
                // Se il player è sul coccodrillo, dobbiamo mantenerlo nella posizione relativa
                if (has_player) {
                    pthread_mutex_lock(&state->player.mutex);
                    state->player.x--;
                    pthread_mutex_unlock(&state->player.mutex);
                }
            }
        }
        
        // Controllo che blocca i proiettili e la possibilità di sparare
        // in caso il gioco si trovi in pausa
        if (!state->game_paused) {
            // Possibilità casuale di sparare un colpo (3%)
            if (rand() % 100 < 3) {
                // Determina la posizione del proiettile in base alla sua direzione
                int bullet_x = (direction > 0) ? 
                    state->crocodiles[id].x + state->crocodiles[id].width - 1 : 
                    state->crocodiles[id].x;
                    
                create_bullet(
                    state,
                    bullet_x,               // x position
                    state->crocodiles[id].y, // y position
                    direction,               // segue la direzione del cocodrillo
                    true                     // colpo del nemico
                );
            }
        }
        
        // Modificare la parte in crocodile_thread che aggiorna lo stato
        // Al posto di aggiornare direttamente state->crocodiles[id]
        
        // Dopo aver calcolato la nuova posizione
        game_message msg;
        msg.type = MSG_CROCODILE;
        msg.id = id;
        msg.pos = state->crocodiles[id]; // Copiare i valori aggiornati
        msg.direction = direction;
        
        // Invia il messaggio invece di aggiornare direttamente
        buffer_put(&state->event_buffer, &msg);
        
        // Rimuovere la manipolazione del player qui, sarà gestita dal game_thread
        
        pthread_mutex_unlock(&state->crocodiles[id].mutex);
        
        // Sleep to control movement speed (using variable speed instead of fixed CROCODILE_SPEED)
        usleep(speed);
    }
    
    // Libera la memoria allocate per i vari arguments
    free(arg);
    return NULL;
}
