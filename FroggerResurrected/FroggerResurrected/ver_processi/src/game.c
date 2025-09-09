#include "../include/game.h"

/*
  o
_`O'_
*/

char rana_sprite[2][5] = {
    {' ', ' ', 'O', ' ', ' '},
    {'_', '`', 'O', '\'', '_'}
    
};


void game(int pipein,int num_coccodrilli)
{
    struct position p;
    srand(time(NULL));
    int pippo;
    //la rana inizia dal centro dello schermo
    struct position rana_pos = {'$', GAME_WIDTH-3, GAME_HEIGHT-2, 2, 5};
    struct position crocodile_positions [num_coccodrilli];

    // Initialize all crocodile positions
    // i coccodrilli si dividono in corsie
    for (int i = 0; i < num_coccodrilli; i++) {
        int lane = i % LANES;  // Distribute across 8 lanes (0-7)
        int x = (i % 2 == 0) ? 1 : GAME_WIDTH - 6;  // Alternate starting from left/right
        
        // Random width between 3-4 times the frog width (frog width = 5)
        int width = (rand() % 2 + 3) * 5;  // Will give either 15 or 20 units
        
        crocodile_positions[i] = (struct position) {
            .c = 'C',
            .x = x,
            .y = 4+(lane*LANE_HEIGHT),
            .width = width,
            .height = 2,  // Keep height same as frog
            .id = i
        };
    }
    
    while (1)
    {
        ssize_t r = read(pipein, &p, sizeof(struct position));
        if (r <= 0) {
            mvprintw(LINES/2, COLS/2-10, "Pipe read error: %s", strerror(errno));
            refresh();
            sleep(1);
            continue;
        }


        // cancello la rana

        for (int i = 0; i < rana_pos.height; i++)
        {
            for (int j = 0; j < rana_pos.width; j++)
            {
                mvaddch(rana_pos.y + i, rana_pos.x + j, ' ');
            }
        }


        //cancello i coccodrilli
        for(int i=0; i<num_coccodrilli; i++){
            for (int h = 0; h < crocodile_positions[i].height; h++) {
                for (int w = 0; w < crocodile_positions[i].width; w++) {
                    mvaddch(crocodile_positions[i].y + h, crocodile_positions[i].x + w, ' ');
                }
            }
        }


            
        //aggiorno la posizione in base al carattere letto
        if (p.c == '$') {
            int crocodile_direction = 0;
            bool was_on_crocodile = rana_coccodrillo(&rana_pos, crocodile_positions, num_coccodrilli, &crocodile_direction);
            if (was_on_crocodile) {
              // Aggiorna la posizione della rana in base all'input del giocatore
                rana_pos.y = p.y;
                // Aggiungi anche il movimento del coccodrillo
                rana_pos.x -= crocodile_direction;
                

            } else {
                rana_pos = p; // Only update position if not on crocodile
            }
            // Controlla che la rana non esca dallo schermo
            if (rana_pos.x < 1) rana_pos.x = 1;
            if (rana_pos.x > GAME_WIDTH - rana_pos.width - 1) 
                rana_pos.x = GAME_WIDTH - rana_pos.width - 1;
        } else if (p.c == 'C') {
            for (int i = 0; i < num_coccodrilli; i++) {
                if (crocodile_positions[i].id == p.id) {
                    crocodile_positions[i] = p;
                    break;
                }
            }
        }


        

        //disegno i coccodrilli
        attron(COLOR_PAIR(2));
        for (int i = 0; i < num_coccodrilli; i++) {
            for (int h = 0; h < crocodile_positions[i].height; h++) {
                for (int w = 0; w < crocodile_positions[i].width; w++) {
                    mvaddch(crocodile_positions[i].y + h, crocodile_positions[i].x + w, 'C');
                }
            }
        }
        attroff(COLOR_PAIR(2));

        //disegno la rana

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
    }
}

bool rana_coccodrillo(struct position *rana_pos, struct position crocodile_positions[], int num_coccodrilli, int *direction) {
    for (int i = 0; i < num_coccodrilli; i++) {
        // Check if frog is on crocodile
        if (rana_pos->y == crocodile_positions[i].y && 
            rana_pos->x >= crocodile_positions[i].x && 
            rana_pos->x <= crocodile_positions[i].x + crocodile_positions[i].width - rana_pos->width) {
            
            // Set the direction based on the crocodile's lane
            int lane = (crocodile_positions[i].id/2) % LANES;
            *direction = (lane % 2 == 0) ? -1 : 1;
            return true; // Frog is on crocodile
        }
    }
    return false; // Frog is not on any crocodile
}