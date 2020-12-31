/*
 * Author: Samuel Ryckman
 * 
 * Class to play music using a motor by varying the frequency of the pwm signal. Only 
 * works on pins 6, 7, and 8. Pins must be used in phase-correct pwm mode in the rest 
 * of the program or this will probably screw things up.
 */
 
#include "MusicPlayer.h"

// Define some notes
#define n_A3 220.0
#define n_B3 246.9
#define n_C4 261.6
#define n_D4 293.7
#define n_E4 329.6
#define n_F4 349.2
#define n_Fs4 370.0
#define n_G4 392.0
#define n_A4 440.0
#define n_B4 493.9
#define n_C5 523.3
#define n_D5 587.3

    
// Define songs
int tempo = 600;
const MusicPlayer::Note wishYouMerryChristmas[] = {
    MusicPlayer::Note(n_D4, tempo*1),
    MusicPlayer::Note(n_G4, tempo*1),
    MusicPlayer::Note(n_G4, tempo*0.5),
    MusicPlayer::Note(n_A4, tempo*0.5),
    MusicPlayer::Note(n_G4, tempo*0.5),
    MusicPlayer::Note(n_Fs4, tempo*0.5),
    MusicPlayer::Note(n_E4, tempo*1),
    MusicPlayer::Note(n_E4, tempo*1),
    MusicPlayer::Note(n_E4, tempo*1),  //We
    MusicPlayer::Note(n_A4, tempo*1),
    MusicPlayer::Note(n_A4, tempo*0.5),
    MusicPlayer::Note(n_B4, tempo*0.5),
    MusicPlayer::Note(n_A4, tempo*0.5),
    MusicPlayer::Note(n_G4, tempo*0.5),
    MusicPlayer::Note(n_Fs4, tempo*1),
    MusicPlayer::Note(n_D4, tempo*1),
    MusicPlayer::Note(n_D4, tempo*1),  //We
    MusicPlayer::Note(n_B4, tempo*1),
    MusicPlayer::Note(n_B4, tempo*0.5),
    MusicPlayer::Note(n_C5, tempo*0.5),
    MusicPlayer::Note(n_B4, tempo*0.5),
    MusicPlayer::Note(n_A4, tempo*0.5),
    MusicPlayer::Note(n_G4, tempo*1),
    MusicPlayer::Note(n_E4, tempo*1),
    MusicPlayer::Note(n_D4, tempo*0.5),
    MusicPlayer::Note(n_D4, tempo*0.5),
    MusicPlayer::Note(n_E4, tempo*1),
    MusicPlayer::Note(n_A4, tempo*1),
    MusicPlayer::Note(n_Fs4, tempo*1),
    MusicPlayer::Note(n_G4, tempo*2),
    MusicPlayer::Note(n_D4, tempo*1), //Good
    MusicPlayer::Note(n_G4, tempo*1),
    MusicPlayer::Note(n_G4, tempo*1),
    MusicPlayer::Note(n_G4, tempo*1),
    MusicPlayer::Note(n_Fs4, tempo*2),
    MusicPlayer::Note(n_Fs4, tempo*1),
    MusicPlayer::Note(n_G4, tempo*1), //you
    MusicPlayer::Note(n_Fs4, tempo*1),
    MusicPlayer::Note(n_E4, tempo*1),
    MusicPlayer::Note(n_D4, tempo*2),
    MusicPlayer::Note(n_A4, tempo*1),
    MusicPlayer::Note(n_B4, tempo*1),
    MusicPlayer::Note(n_A4, tempo*1),
    MusicPlayer::Note(n_G4, tempo*1),
    MusicPlayer::Note(n_D5, tempo*1),
    MusicPlayer::Note(n_D4, tempo*1),
    MusicPlayer::Note(n_D4, tempo*0.5),
    MusicPlayer::Note(n_D4, tempo*0.5),
    MusicPlayer::Note(n_E4, tempo*1),
    MusicPlayer::Note(n_A4, tempo*1),
    MusicPlayer::Note(n_Fs4, tempo*1),
    MusicPlayer::Note(n_G4, tempo*2)
};

/** Constructor for the class.
*/
MusicPlayer::MusicPlayer(bool pwmPin6, bool pwmPin7, bool pwmPin8)
    : pwmPin6(pwmPin6), pwmPin7(pwmPin7), pwmPin8(pwmPin8)
{
    
}

/**
 * Initialize the output
 */
void MusicPlayer::init() {
    TCCR4A = 0;
    TCCR4B = 0;
    TCNT4  = 0;

    // Mode 10: phase correct PWM with ICR4 as Top (= F_CPU/2/25000)
    // OC4C as Non-Inverted PWM output
    ICR4   = (F_CPU/1000)/2;
    if (pwmPin6) {
        OCR4A  = 0;
    }
    if (pwmPin7) {
        OCR4B  = 0;
    }
    if (pwmPin8) {
        OCR4C  = 0;
    }
    TCCR4A = _BV(WGM41);
    if (pwmPin6) {
        TCCR4A  |= _BV(COM4A1);
    }
    if (pwmPin7) {
        TCCR4A  |= _BV(COM4B1);
    }
    if (pwmPin8) {
        TCCR4A  |= _BV(COM4C1);
    }
    TCCR4B = _BV(WGM43) | _BV(CS40);

    // Set the PWM pins as output.
    if (pwmPin6) {
        pinMode(6, OUTPUT);
    }
    if (pwmPin7) {
        pinMode(7, OUTPUT);
    }
    if (pwmPin8) {
        pinMode(8, OUTPUT);
    }
}

/**
 * Play a note at the given frequency and duration with the set volume
 * @param note - note struct with the frequency and duration
 * @param volume - loudness of the note
 */
void MusicPlayer::playNote(Note &note, float volume) {
    Serial.println("Note: [f: "+String(note.frequency)+", d: "+String(note.duration)+"]");
    TCNT4  = 0;
    ICR4   = (F_CPU/note.frequency)/2;
    if (pwmPin6) {
        OCR4A = ICR4*volume;
    }
    if (pwmPin7) {
        OCR4B = ICR4*volume;
    }
    if (pwmPin8) {
        OCR4C = ICR4*volume;
    }
    
    noteTimeStart = millis();
}

/**
 * Play a song.
 * @param notes - array of notes.
 * @numNotes - number of notes in the array.
 * @volume - volume for the song.
 */
void MusicPlayer::playSong(Note notes[], int numNotes, float volume) {
    // Initialize the PWM
    oldICR4 = ICR4;
    init();
    
    // Set the song to playing
    curSong = notes;
    curSongNumNotes = numNotes;
    curSongVolume = volume;
    curNote = 0;
    
    // Play the first note
    playNote(curSong[curNote], curSongVolume);
}

/**
* Continu playing the song. Update position in the song and frequency if time to go to 
* next note. If reach end of song, stop playing.
*/
void MusicPlayer::update() {
    if (isPlaying()) {
        // Check if time to go to next note
        if (millis() - noteTimeStart >= curSong[curNote].duration) {
            curNote++;
            
            // Check if reached end of song
            if (curNote >= curSongNumNotes) {
                stop();
            } else { // Play the next note
                playNote(curSong[curNote], curSongVolume);
            }
        }
    }
}

/**
* Check if there is a song currently playing.
* 
* @return true if song is currently playing, false otherwise.
*/
bool MusicPlayer::isPlaying() {
    return curSong != nullptr;
}

/**
* Stop playing the current song.
*/
void MusicPlayer::stop() {
    // Remove the song from playing
    curSong = nullptr;
    
    // Stop noise and restore initial frequency.
    Note quiteNote(500, 0);
    playNote(quiteNote, 0);
    TCNT4  = 0;
    ICR4 = oldICR4;
}

/**
* Wish the humans a merry Christmas. :)
*/
void MusicPlayer::playMerryChristmas(float volume) {
    playSong(wishYouMerryChristmas, sizeof(wishYouMerryChristmas)/sizeof(Note), volume);
}