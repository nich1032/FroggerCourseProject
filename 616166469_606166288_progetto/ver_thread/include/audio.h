// filepath: /home/matteo/Projects/FroggerResurrected/ver_processi/include/audio.h
#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>

// Tipi di suoni disponibili
typedef enum {
    SOUND_BACKGROUND,
    SOUND_SHOOT,
    SOUND_TANA_ENTER,
    SOUND_SPLASH
} SoundType;

// Riproduci un suono specifico
void play_sound(SoundType sound);

// Avvia/ferma la musica di background
void toggle_background_music(bool play);

// Termina eventuali processi audio in background
void cleanup_audio(void);

#endif // AUDIO_H
