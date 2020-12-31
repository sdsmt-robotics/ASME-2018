/* 1/10/2020
 * Samuel Ryckman
 * 
 * Header for the music player class.
 */

#ifndef MUSIC_PLAYER_H
#define MUSIC_PLAYER_H

#include "Arduino.h"

class MusicPlayer {
public:
    struct Note {
      int frequency;
      int duration;
      Note(int f, int d) : frequency(f), duration(d) {}
    };
    
    MusicPlayer(bool pwmPin6, bool pwmPin7, bool pwmPin8);
  
    void playSong(Note notes[], int numNotes, float volume);
    void playMerryChristmas(float volume);
    
    void stop();
    void update();
    bool isPlaying();
private:
    void init();
    void playNote(Note &note, float volume);
    
    Note* curSong;                  // Current song being played
    int curSongNumNotes;            // Number of notes in the current song
    float curSongVolume;            // Volume for the song currently being played
    int curNote;          // Current note index in the song
    unsigned long noteTimeStart;    // Start time playing the current note
    int oldICR4;                    // PWM timer prescaler before the song started
    
    bool pwmPin6, pwmPin7, pwmPin8;
};

#endif
