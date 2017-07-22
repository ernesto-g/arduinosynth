/**
 *  Arduino Analog-Digital Synth
    Copyright (C) <2017>  Ernesto Gigliotti <ernestogigliotti@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#define MIDI_CURRENT_CHANNEL  0
#define MIDI_CMD_NOTE_ON  144
#define MIDI_CMD_NOTE_OFF  128

#define MIDI_STATE_IDLE       0
#define MIDI_STATE_RVC_DATA1  1
#define MIDI_STATE_RVC_DATA2  2


#define GLISS_STATE_IDLE                0
#define GLISS_STATE_CHANGE_NOTE         1
#define GLISS_STATE_WAIT_NOTE_DURATION  2


#define OCTAVE_MINUS_TWO  0
#define OCTAVE_MINUS_ONE  1
#define OCTAVE_ZERO       2
#define OCTAVE_PLUS_ONE   3
#define OCTAVE_PLUS_TWO   4

#define OUT_MODE0     0
#define OUT_MODE1     1
#define OUT_MODE2     2
#define OUT_MODE3     3
#define OUT_REPEAT    4

#define MIDI_MODE_MONO_KEYS_BOTH_SIDES  0
#define MIDI_MODE_MONO_KEYS_LOW_PRIOR   1
#define MIDI_MODE_DUAL_KEYS_BOTH_SIDES  2
#define MIDI_MODE_SECUENCER             3


typedef struct S_MidiInfo {
  byte channel;
  byte cmd;
  byte note;
  byte vel;
}MidiInfo;

typedef struct S_KeyPressedInfo {
  byte note;
  byte flagFree; 
}KeyPressedInfo;
#define KEYS_PRESSED_LEN    10


void midi_stateMachine(byte midiByte);
void midi_init(void);

void midi_setOctaveVco1(byte octave);
void midi_setOctaveVco2(byte octave);
void midi_setTuneVco1(signed int tuneValue);
void midi_setTuneVco2(signed int tuneValue);
void midi_setRepeatValue(unsigned int repeatVal);
void midi_repeatManager(void);
void midi_glissManager(void);
void midi_setLfoSync(unsigned int val);
void midi_buttonPressedLongCallback(void);
void midi_buttonPressedShortCallback(void);
void midi_startNote(unsigned char midiNoteNumber);
void midi_stopNote(unsigned char midiNoteNumber);
void midi_setGlissOn(unsigned char val);


