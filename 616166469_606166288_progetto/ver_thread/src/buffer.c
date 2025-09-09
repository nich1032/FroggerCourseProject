#include "../include/game.h"
#include <stdlib.h>

void buffer_init(circular_buffer* buf, int capacity) {
    buf->array = malloc(capacity * sizeof(game_message));
    buf->capacity = capacity;
    buf->head = 0;
    buf->tail = 0;
    buf->count = 0;
    pthread_mutex_init(&buf->mutex, NULL);
    pthread_cond_init(&buf->not_full, NULL);
    pthread_cond_init(&buf->not_empty, NULL);
}

void buffer_destroy(circular_buffer* buf) {
    // Controllo che il buffer sia inizializzato
    if (!buf || !buf->array) return;
    
    pthread_mutex_lock(&buf->mutex);
    
    // Se ci sono thread in attesa, segnala a tutti di svegliarsi
    pthread_cond_broadcast(&buf->not_full);
    pthread_cond_broadcast(&buf->not_empty);
    
    pthread_mutex_unlock(&buf->mutex);
    
    // Breve attesa per consentire ai thread di uscire dalle attese
    usleep(50000); // 50ms, aumentato per dare più tempo
    
    // Libera la memoria e distruggi i synchronization primitives
    free(buf->array);
    buf->array = NULL; // Per evitare use-after-free
    buf->capacity = 0;
    buf->count = 0;
    
    pthread_mutex_destroy(&buf->mutex);
    pthread_cond_destroy(&buf->not_full);
    pthread_cond_destroy(&buf->not_empty);
}

void buffer_put(circular_buffer* buf, game_message* item) {
    // Controllo di sicurezza
    if (!buf || !buf->array) return;
    
    pthread_mutex_lock(&buf->mutex);
    
    // Controllo di sicurezza aggiuntivo dopo aver ottenuto il lock
    if (!buf->array) {
        pthread_mutex_unlock(&buf->mutex);
        return;
    }
    
    // Attesa se il buffer è pieno
    while (buf->count == buf->capacity) {
        pthread_cond_wait(&buf->not_full, &buf->mutex);
        
        // Controllo post-wait in caso il buffer sia stato distrutto
        if (!buf->array) {
            pthread_mutex_unlock(&buf->mutex);
            return;
        }
    }
    
    // Aggiunta di un item al buffer
    buf->array[buf->tail] = *item;
    buf->tail = (buf->tail + 1) % buf->capacity;
    buf->count++;
    
    pthread_cond_signal(&buf->not_empty);
    pthread_mutex_unlock(&buf->mutex);
}

void buffer_get(circular_buffer* buf, game_message* msg) {
    // Controllo di sicurezza
    if (!buf || !buf->array) return;
    
    pthread_mutex_lock(&buf->mutex);
    
    // Controllo di sicurezza aggiuntivo dopo aver ottenuto il lock
    if (!buf->array) {
        pthread_mutex_unlock(&buf->mutex);
        return;
    }
    
    // Attesa in caso di buffer vuoto
    while (buf->count == 0) {
        pthread_cond_wait(&buf->not_empty, &buf->mutex);
        
        // Controllo post-wait in caso il buffer sia stato distrutto
        if (!buf->array) {
            pthread_mutex_unlock(&buf->mutex);
            return;
        }
    }
    
    // Presa di un item dal buffer
    *msg = buf->array[buf->head];
    buf->head = (buf->head + 1) % buf->capacity;
    buf->count--;
    
    pthread_cond_signal(&buf->not_full);
    pthread_mutex_unlock(&buf->mutex);
}

bool buffer_try_get(circular_buffer* buf, game_message* msg) {
    // Controllo di sicurezza
    if (!buf || !buf->array) return false;
    
    pthread_mutex_lock(&buf->mutex);
    
    // Controllo di sicurezza aggiuntivo dopo aver ottenuto il lock
    if (!buf->array) {
        pthread_mutex_unlock(&buf->mutex);
        return false;
    }
    
    if (buf->count == 0) {
        pthread_mutex_unlock(&buf->mutex);
        return false;
    }
    
    // Presa di un item dal buffer
    *msg = buf->array[buf->head];
    buf->head = (buf->head + 1) % buf->capacity;
    buf->count--;
    
    pthread_cond_signal(&buf->not_full);
    pthread_mutex_unlock(&buf->mutex);
    return true;
}
