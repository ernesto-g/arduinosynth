#include "Config.h"
#include "MidiManager.h"
#include "Lfo.h"
#include "AnalogIns.h"
#include "FrontPanel.h"
#include "Outs.h"
#include "SequencerManager.h"

void setup() {
  config_init();
  outs_init();
  midi_init();
  lfo_init();
  ain_init();
  frontp_init();
  seq_init();
}


void loop() {
  Serial.print("INIT");
  while(1)
  {
    // MIDI Reception
    byte midiByte;
    if (Serial.available() > 0) 
    {
        midiByte = Serial.read();
        midi_stateMachine(midiByte);
    }
    //_______________    

    ain_state_machine();

    frontp_state_machine();

    midi_repeatManager();

    outs_stateMachine();

    seq_stateMachine();
  }
}
