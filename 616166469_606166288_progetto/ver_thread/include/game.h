#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <ncurses.h>
#include <time.h>
#include <pthread.h>

// Game constants
#define GAME_WIDTH 80
#define GAME_HEIGHT 24
#define FLOOR_HEIGHT 20
#define LANES 8
#define LANE_HEIGHT 2
#define MAX_CROCODILES 16
#define MAX_BULLETS 100
#define NUM_TANE 5
#define TANA_WIDTH 7
#define TANA_HEIGHT 1
#define BUFFER_SIZE 100

// Stato del gioco
typedef struct {
    char c;
    int x;
    int y;
    int width;
    int height;
    int id;
    bool active;
    bool collision;
    pthread_mutex_t mutex;  // Mutex 
} position;

typedef struct {
    int x;
    int y;
    bool occupata;
} tana;

typedef struct {
    position pos;
    int direction;          // 1 for right, -1 for left
    pthread_t thread_id;    
    bool is_enemy;          // true per coccodrillo, false per proiettile
} bullet;

typedef enum {
    MSG_PLAYER,
    MSG_CROCODILE,
    MSG_BULLET
} message_type;

typedef struct {
    message_type type;
    position pos;
    int id;
    int direction;
    bool is_enemy;
} game_message;

typedef struct {
    game_message* array;
    int capacity;
    int head;
    int tail;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
} circular_buffer;

// Game state structure 
typedef struct {
    // Player state
    position player;
    int vite;
    int score;
    
    // Game objects
    position crocodiles[MAX_CROCODILES];
    bullet bullets[MAX_BULLETS];
    tana tane[NUM_TANE];
    
    // Game status
    bool game_over;
    bool game_paused;       // Flag pausa
    int remaining_time;
    int max_time;
    time_t last_update;
    
    // Sincronizzazione
    pthread_mutex_t game_mutex;       // Game state mutex
    pthread_mutex_t screen_mutex;     // Screen drawing mutex
    pthread_cond_t game_update_cond;  // Semaforo per aggiornamento dello stato di gioco
    
    // Stato tane
    int tane_occupate;
    
    // Stato coccodrilli-giocatore  
    bool player_on_crocodile;         
    int player_crocodile_id;          

    circular_buffer event_buffer;
} game_state;

// Thread argument structures
typedef struct {
    game_state* state;
    int id;
} crocodile_args;

typedef struct {
    game_state* state;
    int bullet_id;
} bullet_args;

// Dichiarazione delle funzioni
void* player_thread(void* arg);
void* crocodile_thread(void* arg);
void* bullet_thread(void* arg);
void* game_thread(void* arg);

// Game utility functions
bool rana_coccodrillo(position* rana_pos, position crocodile_positions[], int num_coccodrilli, int* direction);
bool frog_on_the_water(position* rana_pos);
int find_free_bullet_slot(game_state* state);
void create_bullet(game_state* state, int x, int y, int direction, bool is_enemy);

// Funzioni di inizializzazione e distruzione
void init_game_state(game_state* state);
void destroy_game_state(game_state* state);

extern char rana_sprite[2][5];

#endif // GAME_H