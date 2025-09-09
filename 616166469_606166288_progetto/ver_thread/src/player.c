#include "../include/game.h"
#include "../include/audio.h" // Added
#include <unistd.h>
#include <stdlib.h>

void* player_thread(void* arg) {
    game_state* state = (game_state*)arg;
    
    while (!state->game_over) {
        int ch = getch();
        if (ch == ERR) {
            usleep(20000);
            continue;
        }
        
        // Blocco per il controllo della pausa
        if (ch == 'p' || ch == 'P') {
            pthread_mutex_lock(&state->game_mutex);
            state->game_paused = !state->game_paused;
            
            // Messaggio di pausa
            if (state->game_paused) {
                pthread_mutex_lock(&state->screen_mutex);
                mvprintw(LINES/2, COLS/2-10, "GIOCO IN PAUSA");
                mvprintw(LINES/2+1, COLS/2-15, "Premi 'p' per continuare");
                refresh();
                pthread_mutex_unlock(&state->screen_mutex);
            } else {
                state->last_update = time(NULL);
                
                draw_game_state(state);
            }
            
            pthread_mutex_unlock(&state->game_mutex);
            continue; 
        }
        
        //controllo pausa
        if (state->game_paused) {
            usleep(50000);
            continue;
        }
        
        // Blocco per aggiornare la posizione del giocatore
        pthread_mutex_lock(&state->player.mutex);
        
        int prev_x = state->player.x;
        int prev_y = state->player.y;
        bool was_on_crocodile = state->player_on_crocodile;
        
        switch (ch) {
            case KEY_UP:
                if (state->player.y > 1) {
                    state->player.y -= state->player.height;
                }
                break;
            case KEY_DOWN:
                if (state->player.y < LINES - 2 - state->player.height + 1) {
                    state->player.y += state->player.height;
                }
                break;
            case KEY_LEFT:
                if (state->player.x > 1) {
                    state->player.x -= 2;
                }
                break;
            case KEY_RIGHT:
                if (state->player.x < COLS - 2 - state->player.width + 1) {
                    state->player.x += 2;
                }
                break;
            case ' ': 
                create_bullet(
                    state, 
                    state->player.x - 1,       
                    state->player.y,           
                    -1,                        
                    false                      
                );
                
                // Create right bullet
                create_bullet(
                    state, 
                    state->player.x + state->player.width, 
                    state->player.y,                       
                    1,                                     
                    false                                
                );
                play_sound(SOUND_SHOOT); // Added sound for shooting
                break;
                
            case 'q':  // Cheat code per il salto
                state->player.x = GAME_WIDTH/2;
                state->player.y = 2;
                break;
                
            case 'Q':  // Quit game
                pthread_mutex_lock(&state->game_mutex);
                state->game_over = true;
                pthread_mutex_unlock(&state->game_mutex);
                break;
        }
        
        // Controllo dei bordi
        if (state->player.x < 1) 
            state->player.x = 1;
        if (state->player.x > GAME_WIDTH - state->player.width - 1) 
            state->player.x = GAME_WIDTH - state->player.width - 1;
        
        // Controllo movimento laterale
        if ((prev_x != state->player.x || prev_y != state->player.y) && 
            was_on_crocodile) {
            state->player_on_crocodile = false;
            state->player_crocodile_id = -1;
        }
        
        game_message msg;
        msg.type = MSG_PLAYER;
        msg.id = 0; // Player ID è sempre 0
        msg.pos = state->player; // Copiare i valori aggiornati
        buffer_put(&state->event_buffer, &msg);

        pthread_mutex_unlock(&state->player.mutex);
        
        usleep(50000);
    }
    
    return NULL;
}