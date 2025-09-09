#include "../include/utils.h"


void draw_score(int score) {
    
    int x_start = COLS - 12; 
    int y_start = LINES - 1; 

    mvprintw(y_start, x_start, "SCORE: %d", score);
    refresh();
}

void draw_time_bar(int remaining_time, int max_time) {

    int bar_length = 20;  // Lunghezza della barra
    int filled_length = (remaining_time * bar_length) / max_time; // Calcola quanto riempire
    int x_start = 2;      
    int y_start = LINES - 1; 
    
    // Stampa etichetta
    mvprintw(y_start, x_start, "TEMPO: ");

    // Disegna barra piena (verde)
    attron(COLOR_PAIR(1)); 

    for (int i = 0; i < filled_length; i++) {
        mvaddch(y_start, x_start + 8 + i, ACS_CKBOARD);
    }
    attroff(COLOR_PAIR(1));

    // Disegna barra vuota
    for (int i = filled_length; i < bar_length; i++) {
        mvaddch(y_start, x_start + 8 + i, ' ');
    }
}


void draw_river_borders() {
    attron(COLOR_PAIR(3)); // Nuovo colore per i bordi del fiume
    
    // Disegna bordo superiore del fiume (y=3)
    mvhline(3, 1, ACS_HLINE, GAME_WIDTH-2);
    mvaddch(3, 0, ACS_LTEE);
    mvaddch(3, GAME_WIDTH-1, ACS_RTEE);
    
    // Disegna bordo inferiore del fiume (y=FLOOR_HEIGHT)
    mvhline(FLOOR_HEIGHT, 1, ACS_HLINE, GAME_WIDTH-2);
    mvaddch(FLOOR_HEIGHT, 0, ACS_LTEE);
    mvaddch(FLOOR_HEIGHT, GAME_WIDTH-1, ACS_RTEE);
    
    attroff(COLOR_PAIR(3));
}

void clear_frog_position(struct position *pos) {
    for (int i = 0; i < pos->height; i++) {
        for (int j = 0; j < pos->width; j++) {
            // Se siamo su un bordo del fiume, ridisegna il bordo
            if (pos->y + i == 3 || pos->y + i == FLOOR_HEIGHT) {
                mvaddch(pos->y + i, pos->x + j, ACS_HLINE);
            } else {
                mvaddch(pos->y + i, pos->x + j, ' ');
            }
        }
    }
}

void draw_game_borders() {
    // Colore per la terra (marrone)
    init_pair(5, COLOR_YELLOW, COLOR_RED);
    
    // Disegna la terra di partenza (bottom)
    attron(COLOR_PAIR(5));
    for (int x = 1; x < GAME_WIDTH-1; x++) {
        for (int y = FLOOR_HEIGHT+1; y < GAME_HEIGHT-1; y++) {
            mvaddch(y, x, ACS_CKBOARD);
        }
    }
    
    // Disegna la tana di arrivo (top)
    for (int x = 1; x < GAME_WIDTH-1; x++) {
        for (int y = 1; y < 3; y++) {
            mvaddch(y, x, ACS_CKBOARD);
        }
    }
    attroff(COLOR_PAIR(5));
}


void init_dens(struct tana *tane) {
    int usable_width = GAME_WIDTH - 2; 
    int den_space = (usable_width - (NUM_TANE * TANA_WIDTH)) / (NUM_TANE + 1);
    int current_x = 1; 
    
    for(int i = 0; i < NUM_TANE; i++) {
        current_x += den_space; // aggiungo spacing
        tane[i].x = current_x;
        tane[i].y = 1;  // Limite superiore
        tane[i].occupata = false;
        current_x += TANA_WIDTH;
    }
}

void draw_dens(struct tana tane[]) {
    for(int i = 0; i < NUM_TANE; i++) {
        // Disegna bordo della tana
        attron(COLOR_PAIR(3)); // Usa colore del bordo
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
bool check_den_collision(struct position *rana_pos, struct tana tane[]) {
    // Punto centrale della rana
    int frog_center_x = rana_pos->x + (rana_pos->width / 2);
    
    for(int i = 0; i < NUM_TANE; i++) {
        // controlla se la tana è occupata
        if(!tane[i].occupata) {
            if(frog_center_x >= tane[i].x && 
               frog_center_x <= tane[i].x + TANA_WIDTH &&
               rana_pos->y <= tane[i].y + TANA_HEIGHT) {
                tane[i].occupata = true;
                return true;
            }
        }
    }
    return false;
}

// Pulizia centralizzata per gli oggetti di gioco
void clear_entities(struct position *rana_pos, struct position crocodile_positions[], int num_coccodrilli, struct position bullets[], int max_bullets) {
    // Clear frog
    clear_frog_position(rana_pos);

    // Clear crocodiles
    for(int i=0; i < num_coccodrilli; i++) {
        for (int h = 0; h < crocodile_positions[i].height; h++) {
            for (int w = 0; w < crocodile_positions[i].width; w++) {
                mvaddch(crocodile_positions[i].y + h, crocodile_positions[i].x + w, ' ');
            }
        }
    }

    // Clear bullets
    for (int i = 0; i < max_bullets; i++) {
        if(bullets[i].active) {
            mvaddch(bullets[i].y, bullets[i].x, ' ');
        }
    }
}

void draw_frog(struct position *rana_pos) {
    attron(COLOR_PAIR(1));
    for (int i = 0; i < rana_pos->height; i++) {
        for (int j = 0; j < rana_pos->width; j++) {
            mvaddch(rana_pos->y + i, rana_pos->x + j, rana_sprite[i][j]);
        }
    }
    attroff(COLOR_PAIR(1));
}

void draw_crocodiles(struct position crocodile_positions[], int num_coccodrilli) {
    attron(COLOR_PAIR(2));
    for (int i = 0; i < num_coccodrilli; i++) {
        int lane = (crocodile_positions[i].id/2) % LANES;
        int direction = (lane % 2 == 0) ? 1 : -1;
        
        for (int h = 0; h < crocodile_positions[i].height; h++) {
            for (int w = 0; w < crocodile_positions[i].width && w < 15; w++) {
                if (direction > 0) {
                    mvaddch(crocodile_positions[i].y + h, crocodile_positions[i].x + w, crocodile_sprite_dx[h][w]);
                } else {
                    mvaddch(crocodile_positions[i].y + h, crocodile_positions[i].x + w, crocodile_sprite_sx[h][w]);
                }
            }
        }
    }
    attroff(COLOR_PAIR(2));
}

void draw_bullets(struct position bullets[]) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active && !bullets[i].collision) {
            mvaddch(bullets[i].y, bullets[i].x, bullets[i].c);
        }
    }
}


//Controllo se la rana è in un'area non valida
// (cioè non in una tana libera)
bool is_invalid_top_area(struct position *rana_pos, struct tana tane[]) {
    if (rana_pos->y <= 1) {
        int frog_center_x = rana_pos->x + (rana_pos->width / 2);
        for (int i = 0; i < NUM_TANE; i++) {
            if (!tane[i].occupata && 
                frog_center_x >= tane[i].x && 
                frog_center_x <= tane[i].x + TANA_WIDTH) {
                // La rana è in una tana libera
                return false;
            }
        }
        // La rana non è in una tana libera
        return true;
    }
    // La rana non è in un'area non valida
    return false;
}
