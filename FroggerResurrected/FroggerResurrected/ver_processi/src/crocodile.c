#include "../include/crocodile.h"

#include <time.h>

/*I coccodrilli si dividono in corsie in base al loro id */

void coccodrillo(int pipeout,int id){
    srand(time(NULL)^id);
    struct position p;
    int direction = 0; // 1 = right, -1 = left
    int original_width;
    
    //inizializzo i valori del coccodrillo
    p.c = 'C';
    p.id = id;
    p.width = (rand() % 2 +2) *5;
    original_width = p.width;
    p.height = 2;

    //divido i coccodrilli in corsie
    int lane = (id/2) % LANES;

    direction = (lane % 2 == 0) ? 1 : -1; //alternare la direzione di partenza

    int is_second= id % 2;

    if (is_second ){
        p.x = (GAME_WIDTH/2) + (rand() % (GAME_WIDTH/4));
    }else{
        p.x =rand() % (GAME_WIDTH/4);
    }

    p.y = 4+(lane*LANE_HEIGHT);

    while (1)
    {
        handle_border_collision(&p, &original_width);
        update_position(&p, direction);

        //scrivo la posizione nella pipe
        write(pipeout, &p, sizeof(struct position));

        usleep(200000);
    }
    

}

 void handle_border_collision(struct position *p, int *original_width) {
    if (p->x <= 1) {
        p->x = 2;
        p->width = p->width - 1;
        if(p->width <= 0) {
            p->width = *original_width;
            p->x = GAME_WIDTH-2;
        }
    } else if (p->x >= GAME_WIDTH-3) {
        p->x = GAME_WIDTH-3;
        p->width = p->width - 1;
        if (p->width <= 0) {
            p->width = *original_width;
            p->x = 1;
        }
    }
}

 void update_position(struct position *p, int direction) {
    p->x += direction;
}