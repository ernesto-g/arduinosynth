#define MIDI_CURRENT_CHANNEL  0
#define MIDI_CMD_NOTE_ON  144
#define MIDI_CMD_NOTE_OFF  128

#define MIDI_STATE_IDLE       0
#define MIDI_STATE_RVC_DATA1  1
#define MIDI_STATE_RVC_DATA2  2

#define MIDI_VOICES_MODE_MONO 0
#define MIDI_VOICES_MODE_DUAL 1


typedef struct S_MidiInfo {
  byte channel;
  byte cmd;
  byte note;
  byte vel;
}MidiInfo;


void midi_stateMachine(byte midiByte);
void midi_init(void);

