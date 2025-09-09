#include "../include/utils.h"

//Sprite per il movimento del coccodrillo verso sinistra o destra
char crocodile_sprite_sx[2][15] = {
    {' ', '_', '_', '_', '/', '^', '\\', '_', '_', '_', '_', '_', '_', '_', '_'},
    {'/', '=', '=', '=', '=', '=', '=', '=', '=', '=', '=', '=', '=', '=', '='}
};

char crocodile_sprite_dx[2][15] = {
    {'_', '_', '_', '_', '_', '_', '_', '/', '^', '\\', '_', '_', '_', '_', ' '},
    {'=', '=', '=', '=', '=', '=', '=', '=', '=', '=', '=', '=', '=', '=', '\\'}
};

void safe_mvaddch(int y, int x, chtype ch, pthread_mutex_t* screen_mutex) {
    pthread_mutex_lock(screen_mutex);
    mvaddch(y, x, ch);
    pthread_mutex_unlock(screen_mutex);
}

void draw_score(int score) {
    int x_start = COLS - 12; 
    int y_start = LINES - 1; 

    mvprintw(y_start, x_start, "SCORE: %d", score);
}

void draw_time_bar(int remaining_time, int max_time) {
    int bar_length = 20;
    int filled_length = (remaining_time * bar_length) / max_time;
    int x_start = 2;      
    int y_start = LINES - 1; 
    
    // Label
    mvprintw(y_start, x_start, "TEMPO: ");

    // Disegna la parte piena della barra (verde)
    attron(COLOR_PAIR(1)); 
    for (int i = 0; i < filled_length; i++) {
        mvaddch(y_start, x_start + 8 + i, ACS_CKBOARD);
    }
    attroff(COLOR_PAIR(1));

    // Disegna la parte vuota
    for (int i = filled_length; i < bar_length; i++) {
        mvaddch(y_start, x_start + 8 + i, ' ');
    }
}

void draw_river_borders() {
    attron(COLOR_PAIR(3));
    
    // Bordo superiore del fiume (y=3)
    mvhline(3, 1, ACS_HLINE, GAME_WIDTH-2);
    mvaddch(3, 0, ACS_LTEE);
    mvaddch(3, GAME_WIDTH-1, ACS_RTEE);
    
    // Bordo inferiore del fiume (y=FLOOR_HEIGHT)
    mvhline(FLOOR_HEIGHT, 1, ACS_HLINE, GAME_WIDTH-2);
    mvaddch(FLOOR_HEIGHT, 0, ACS_LTEE);
    mvaddch(FLOOR_HEIGHT, GAME_WIDTH-1, ACS_RTEE);
    
    attroff(COLOR_PAIR(3));
}

void clear_frog_position(position *pos) {
    if (!pos->active) return;
    
    for (int i = 0; i < pos->height; i++) {
        for (int j = 0; j < pos->width; j++) {
            // QUando ci si trova su un bordo del fiume si ridisegna il bordo
            if (pos->y + i == 3 || pos->y + i == FLOOR_HEIGHT) {
                mvaddch(pos->y + i, pos->x + j, ACS_HLINE);
            } else {
                mvaddch(pos->y + i, pos->x + j, ' ');
            }
        }
    }
}

void draw_game_borders() {
    // Marrone per la terra
    init_pair(5, COLOR_YELLOW, COLOR_RED);
    
    // Disegna zona di terra inferiore
    attron(COLOR_PAIR(5));
    for (int x = 1; x < GAME_WIDTH-1; x++) {
        for (int y = FLOOR_HEIGHT+1; y < GAME_HEIGHT-1; y++) {
            mvaddch(y, x, ACS_CKBOARD);
        }
    }
    
    // Disegna la zona di terra superiore
    for (int x = 1; x < GAME_WIDTH-1; x++) {
        for (int y = 1; y < 3; y++) {
            mvaddch(y, x, ACS_CKBOARD);
        }
    }
    attroff(COLOR_PAIR(5));
}

void init_dens(tana tane[]) {
    int usable_width = GAME_WIDTH - 2;
    int den_space = (usable_width - (NUM_TANE * TANA_WIDTH)) / (NUM_TANE + 1);
    int current_x = 1;
    
    for(int i = 0; i < NUM_TANE; i++) {
        current_x += den_space;
        tane[i].x = current_x;
        tane[i].y = 1;
        tane[i].occupata = false;
        current_x += TANA_WIDTH;
    }
}

void draw_dens(tana tane[]) {
    for(int i = 0; i < NUM_TANE; i++) {
        // Disegna il bordo della tana
        attron(COLOR_PAIR(3));
        mvaddch(tane[i].y, tane[i].x - 1, ACS_LTEE);
        mvaddch(tane[i].y, tane[i].x + TANA_WIDTH, ACS_RTEE);
        attroff(COLOR_PAIR(3));

        if(tane[i].occupata) {
            // Disegna tana occupata
            attron(COLOR_PAIR(7));
            for(int w = 0; w < TANA_WIDTH; w++) {
                mvaddch(tane[i].y, tane[i].x + w, ACS_CKBOARD);
            }
            attroff(COLOR_PAIR(7));
        } else {
            // Disegna tana vuota
            attron(COLOR_PAIR(6));
            for(int w = 0; w < TANA_WIDTH; w++) {
                mvaddch(tane[i].y, tane[i].x + w, ' ');
            }
            attroff(COLOR_PAIR(6));
        }
    }
}

// Disegna l'intero gaem state in maniera atomica con doppio buffering
void draw_game_state(game_state* state) {
    static WINDOW* buffer_win = NULL;
    
    pthread_mutex_lock(&state->screen_mutex);
    
    // Inizializza il buffer solo una volta
    if (buffer_win == NULL) {
        buffer_win = newpad(GAME_HEIGHT, GAME_WIDTH);
    }
    
    // Inizializza il buffer prima di disegnare
    werase(buffer_win);  // Uso werase invece di wclear per migliori performance
    
    // Disegna elementi di sfondo nel buffer
    box(buffer_win, ACS_VLINE, ACS_HLINE);
    
    // Disegna i bordi in buffer_win
    wattron(buffer_win, COLOR_PAIR(3));
    mvwhline(buffer_win, 3, 1, ACS_HLINE, GAME_WIDTH-2);
    mvwaddch(buffer_win, 3, 0, ACS_LTEE);
    mvwaddch(buffer_win, 3, GAME_WIDTH-1, ACS_RTEE);
    mvwhline(buffer_win, FLOOR_HEIGHT, 1, ACS_HLINE, GAME_WIDTH-2);
    mvwaddch(buffer_win, FLOOR_HEIGHT, 0, ACS_LTEE);
    mvwaddch(buffer_win, FLOOR_HEIGHT, GAME_WIDTH-1, ACS_RTEE);
    wattroff(buffer_win, COLOR_PAIR(3));
    
    // DIsegna i bordi di gioco in buffer_win
    wattron(buffer_win, COLOR_PAIR(5));
    for (int x = 1; x < GAME_WIDTH-1; x++) {
        for (int y = FLOOR_HEIGHT+1; y < GAME_HEIGHT-1; y++) {
            mvwaddch(buffer_win, y, x, ACS_CKBOARD);
        }
    }
    for (int x = 1; x < GAME_WIDTH-1; x++) {
        for (int y = 1; y < 3; y++) {
            mvwaddch(buffer_win, y, x, ACS_CKBOARD);
        }
    }
    wattroff(buffer_win, COLOR_PAIR(5));
    
    // Disegna le tane in buffer_win
    for(int i = 0; i < NUM_TANE; i++) {
        wattron(buffer_win, COLOR_PAIR(3));
        mvwaddch(buffer_win, state->tane[i].y, state->tane[i].x - 1, ACS_LTEE);
        mvwaddch(buffer_win, state->tane[i].y, state->tane[i].x + TANA_WIDTH, ACS_RTEE);
        wattroff(buffer_win, COLOR_PAIR(3));

        if(state->tane[i].occupata) {
            wattron(buffer_win, COLOR_PAIR(7));
            for(int w = 0; w < TANA_WIDTH; w++) {
                mvwaddch(buffer_win, state->tane[i].y, state->tane[i].x + w, ACS_CKBOARD);
            }
            wattroff(buffer_win, COLOR_PAIR(7));
        } else {
            wattron(buffer_win, COLOR_PAIR(6));
            for(int w = 0; w < TANA_WIDTH; w++) {
                mvwaddch(buffer_win, state->tane[i].y, state->tane[i].x + w, ' ');
            }
            wattroff(buffer_win, COLOR_PAIR(6));
        }
    }
    
    // Disegna i coccodrilli su buffer_win
    wattron(buffer_win, COLOR_PAIR(2));
    for (int i = 0; i < MAX_CROCODILES; i++) {
        if (state->crocodiles[i].active) {
            pthread_mutex_lock(&state->crocodiles[i].mutex);

            //si decide quale sprite utilizzare a seconda della direzione
            char (*sprite)[15];  // Puntatore a un array di caratteri
        if ((i / 2) % 2 == 0) { 
            sprite = crocodile_sprite_dx; // Muove a destra
        } else {
            sprite = crocodile_sprite_sx; // Muove a sinistra
        }
            
            for (int h = 0; h < state->crocodiles[i].height; h++) {
                for (int w = 0; w < state->crocodiles[i].width; w++) {
                    mvwaddch(buffer_win, state->crocodiles[i].y + h, 
                           state->crocodiles[i].x + w, sprite[h][w]); //stampa sprite
                }
            }
            pthread_mutex_unlock(&state->crocodiles[i].mutex);
        }
    }
    wattroff(buffer_win, COLOR_PAIR(2));
    
    // Disegna i proiettili nel buffer
    for (int i = 0; i < MAX_BULLETS; i++) {
        pthread_mutex_lock(&state->bullets[i].pos.mutex);
        if (state->bullets[i].pos.active && !state->bullets[i].pos.collision) {
            // Disegna solo proiettili attivi e senza collisioni
            mvwaddch(buffer_win, state->bullets[i].pos.y, 
                   state->bullets[i].pos.x, 
                   state->bullets[i].is_enemy ? '@' : '*');
        }
        pthread_mutex_unlock(&state->bullets[i].pos.mutex);
    }
    
    // Disegna il giocatore nel buffer
    wattron(buffer_win, COLOR_PAIR(1));
    pthread_mutex_lock(&state->player.mutex);
    if (state->player.active) {
        for (int i = 0; i < state->player.height; i++) {
            for (int j = 0; j < state->player.width; j++) {
                mvwaddch(buffer_win, state->player.y + i, 
                       state->player.x + j, 
                       rana_sprite[i][j]);
            }
        }
    }
    pthread_mutex_unlock(&state->player.mutex);
    wattroff(buffer_win, COLOR_PAIR(1));
    
    // Status info on buffer
    mvwprintw(buffer_win, LINES-1, COLS - 12, "SCORE: %d", state->score);
    mvwprintw(buffer_win, LINES-1, GAME_WIDTH-20, "Vite: %d", state->vite);
    
    // Disegna il tempo nel buffer
    int bar_length = 20;
    int filled_length = (state->remaining_time * bar_length) / state->max_time;
    int x_start = 2;      
    int y_start = LINES - 1;
    
    mvwprintw(buffer_win, y_start, x_start, "TEMPO: ");
    wattron(buffer_win, COLOR_PAIR(1)); 
    for (int i = 0; i < filled_length; i++) {
        mvwaddch(buffer_win, y_start, x_start + 8 + i, ACS_CKBOARD);
    }
    wattroff(buffer_win, COLOR_PAIR(1));
    for (int i = filled_length; i < bar_length; i++) {
        mvwaddch(buffer_win, y_start, x_start + 8 + i, ' ');
    }
    
    // Non usiamo clear() qui, fa sfarfallare lo schermo
    
    prefresh(buffer_win, 0, 0, 0, 0, GAME_HEIGHT-1, GAME_WIDTH-1);
    
    pthread_mutex_unlock(&state->screen_mutex);
}
