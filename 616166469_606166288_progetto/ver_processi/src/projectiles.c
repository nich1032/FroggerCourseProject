#include "../include/projectiles.h"

void bullet(int pipeout, struct position *p, int direction)
{
    struct position bullet = {
        .c = p->c,
        .x = p->x,
        .y = p->y,
        .width = 1,
        .height = 1,
        .active = 1,
        .pid = getpid(),
        .collision = 0
    };

    while(bullet.active) {
        bullet.x += direction;
        
        if(bullet.x >= GAME_WIDTH-2 || bullet.x <= 1) {
            bullet.active = 0;
            
            write(pipeout, &bullet, sizeof(struct position));

            close(pipeout);
            _exit(0);
        }
        
        write(pipeout, &bullet, sizeof(struct position));
        usleep(50000);
    }
    
    close(pipeout);
    _exit(0);
}