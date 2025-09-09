// filepath: /home/matteo/Projects/FroggerResurrected/ver_processi/src/audio.c
#include "../include/audio.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

static pid_t bg_music_pid = -1;

static void stop_background_music(void) {
    if (bg_music_pid > 0) {
        kill(bg_music_pid, SIGKILL);
        waitpid(bg_music_pid, NULL, 0);
        bg_music_pid = -1;
    }
}

void play_sound(SoundType sound) {
    const char* sound_files[] = {
        "background.wav",     // SOUND_BACKGROUND
        "blip.wav",           // SOUND_SHOOT
        "winning.wav",        // SOUND_TANA_ENTER
        "fall.wav",           // SOUND_SPLASH
    };
    
    if (sound > SOUND_BACKGROUND && sound < (sizeof(sound_files) / sizeof(sound_files[0]))) {
        char command[300];
        snprintf(command, sizeof(command), "aplay ../audio/%s 2>/dev/null &", sound_files[sound]);
        system(command);
    }
}

void toggle_background_music(bool play) {
    if (play) {
        if (bg_music_pid == -1) {
            bg_music_pid = fork();
            if (bg_music_pid == 0) {
                freopen("/dev/null", "w", stdout);
                freopen("/dev/null", "w", stderr);
                
                char* const argv_shell[] = {"sh", "-c", "while true; do aplay ../audio/background.wav; sleep 0.1; done", NULL};
                execvp("sh", argv_shell);
                
                perror("execvp for background music shell failed");
                _exit(127); 
            } else if (bg_music_pid < 0) {
                perror("fork for background music failed");
                bg_music_pid = -1; 
            }
        }
    } else {
        stop_background_music();
    }
}

void cleanup_audio(void) {
    stop_background_music();
}
